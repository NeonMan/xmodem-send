#include "transfer.h"
#include "crc16-xmodem.h"
#include <QSerialPortInfo>
#include <QIODevice>
#include <QDebug>
#include <QFile>
#include <QByteArray>

#define XMODEM_ACK  ((char)0x06)
#define XMODEM_NACK ((char)0x15)
#define XMODEM_CRC  'C'
#define XMODEM_SOH  ((char)0x01)
#define XMODEM_EOT  ((char)0x04)

static char xmodem_sum(const QByteArray &ba){
    char rv = 0;
    for(char b : ba){
        rv = rv + b;
    }
    return rv;
}

Transfer::Transfer(QString serialPortName, qint32 baudrate, QString filePath, QObject *parent)  : QThread(parent)
{
    qDebug() << __FILE__ << __LINE__ << "--" << __func__;
    this->cancelRequested = false;
    this->filePath = filePath;
    this->serialPort = nullptr;
    //Find the serial port by name
    for(QSerialPortInfo &port_info : QSerialPortInfo::availablePorts()){
        if(port_info.portName() == serialPortName){
            this->serialPort = new QSerialPort(port_info, this);
        }
    }
    //If port found, configure it
    if(this->serialPort){
        this->serialPort->setBaudRate(baudrate);
        this->serialPort->setDataBits(QSerialPort::DataBits::Data8); //<-- Always 8

        //ToDo: These serial parameters should come from ui instead of hardcoded values
        this->serialPort->setParity(QSerialPort::Parity::NoParity);
        this->serialPort->setStopBits(QSerialPort::StopBits::OneStop);
        this->serialPort->setFlowControl(QSerialPort::FlowControl::NoFlowControl); //<-- Either NONE or HARWARE
    }
}

Transfer::~Transfer(){
    qDebug() << __FILE__ << __LINE__ << "--" << __func__;
}

void Transfer::launch(){
    qDebug() << __FILE__ << __LINE__ << "--" << __func__;

    //Launch worker thread
    this->start();
}

void Transfer::cancel(){
    qDebug() << __FILE__ << __LINE__ << "--" << __func__;
    emit transferFailed(tr("Transfer cancelled"));
}


//The bulk of the work goes here
void Transfer::run(){
    qDebug() << __FILE__ << __LINE__ << "--" << __func__;

    //Reset progress
    emit updateProgress(0.0f);

    //Open serial port
    this->serialPort->open(QIODevice::ReadWrite);
    if(!this->serialPort->isOpen()){
        emit transferFailed(tr("Unable to open port: ") + this->serialPort->portName());
        this->cancelRequested = true;
    }

    //Open input file
    QFile *in_file = new QFile(this->filePath);
    if(in_file){
        in_file->open(QIODevice::ReadOnly);
        if(!in_file->isOpen()){
            emit transferFailed(tr("Unable to open file: ") + this->filePath);
            this->cancelRequested = true;
        }
    }
    else{
        emit transferFailed(tr("Unable to open QFile: ") + this->filePath);
        this->cancelRequested = true;
    }

    //If cancel requested at this stage, just cleanup early and return
    if(cancelRequested){
        if(in_file){ delete in_file; }
        this->serialPort->close();
        return;
    }

    //Initialize transfer status
    quint32 current_packet = 1;
    bool use_crc = true;

    //Transfer loop
    bool transferComplete = false;
    do{
        //Wait for the first character from recipient
        char status_char = '\0';
        if(this->serialPort->waitForReadyRead(this->timeoutFirstRead)){
            this->serialPort->read(&status_char, 1);
            //Received a byte
            if(status_char == XMODEM_CRC){
                use_crc = true;
                status_char = XMODEM_NACK;
            }
            else if(status_char == XMODEM_NACK){
                use_crc = false;
            }
            else{
                emit transferFailed(tr("Incorrect start of XMODEM transmission (0x") + QString::number(status_char,16) + ")");
                this->cancelRequested = true;
                continue; //<-- Will cleanup and return
            }
        }
        else{
            //Timeout
            emit transferFailed(tr("Timeout"));
            this->cancelRequested = true;
            continue; //<-- Will cleanup and return
        }

        //The proper, real, main XMODEM loop
        while(!cancelRequested && !transferComplete){
            QByteArray packet;

            if(!in_file->atEnd())
            {
                //Packet header
                packet.append(XMODEM_SOH);
                packet.append(current_packet & 0xFF);
                packet.append(255U - (current_packet & 0xFF));

                //Payload
                QByteArray payload = in_file->read(128);
                if (payload.length() < 128){
                    int padding = 128 - payload.length();
                    while(padding){
                        payload.append((char)0x00);
                        padding--;
                    }
                }
                packet.append(payload);

                //Checksum
                if(use_crc){
                    uint16_t packet_crc = crc_init();
                    packet_crc = crc_update(packet_crc, payload.data(), payload.size());
                    packet_crc = crc_finalize(packet_crc);

                    //Add CRC, big endian.
                    packet.append((packet_crc >> 8) & 0xFF);
                    packet.append(packet_crc & 0xFF);
                }
                else{
                    packet.append(xmodem_sum(packet));
                }
            }
            else{
                //Transfer complete!
                packet.append(XMODEM_EOT);
                packet.append(XMODEM_EOT);
                packet.append(XMODEM_EOT);
                transferComplete = true;
            }
            this->serialPort->write(packet);
            this->serialPort->waitForBytesWritten();

            //Read receiver response
            if(this->serialPort->waitForReadyRead(timeoutRead)){
                this->serialPort->read(&status_char, 1);
                if(status_char == XMODEM_ACK){
                    current_packet++;
                }
                else{
                    in_file->seek((current_packet - 1) * 128);
                }
            }
            else{
                emit transferFailed(tr("Status timeout"));
                cancelRequested = true;
            }

            //Update with transfer progress
            emit updateProgress( ((current_packet - 1) * 128.0f) / ((float) in_file->size()) );

        }
    }while(0); //Shameless goto bait

    //Transfer completed, do cleanup
    if(in_file){
        delete in_file;
    }
    this->serialPort->close();
    emit updateProgress(1.0f);
    if(transferComplete && !cancelRequested){
        emit transferCompleted();
    }
}
