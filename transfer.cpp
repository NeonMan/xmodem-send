/* Copyright (C) 2020 J.Luis <root@heavydeck.net>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */
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

Transfer::Transfer(
        QString serialPortName,
        qint32 baudrate,
        QString filePath,
        QObject *parent
        )  : QThread(parent)
{
    qDebug() << __FILE__ << __LINE__ << "--" << __func__;
    this->cancelRequested = false;
    this->filePath = filePath;
    this->serialPort = nullptr;
    this->usePkcsPadding = false;
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

void Transfer::setParity(QSerialPort::Parity parirty){
    this->serialPort->setParity(parirty);
}

void Transfer::setStopBits(QSerialPort::StopBits stopBits){
    this->serialPort->setStopBits(stopBits);
}

void Transfer::SetFlowControl(QSerialPort::FlowControl flowControl){
    this->serialPort->setFlowControl(flowControl);
}

void Transfer::setPkcsPadding(bool enabled){
    this->usePkcsPadding = enabled;
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
    bool sendPkcsPacket = ((in_file->size() % 128) == 0) && this->usePkcsPadding;
    bool pkcsPacketSent = false;
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
                    char pad_byte = padding & 0xFF;

                    //Padding of half-full packets will be performed using the PKCS#7
                    //method of filling the pachet with the value fo the gap size.
                    //If the file to be transfered is an even split of 128 bytes *and*
                    //PKCS#7 flag is enabled (this->usePkcsPadding) an aditional 128 byte packet
                    //of the number 128 repeated all over it *must* be sent as to guarantee
                    //padding is always present.
                    while(padding){
                        payload.append((char)pad_byte);
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
            else if (sendPkcsPacket){
                //Packet header
                packet.append(XMODEM_SOH);
                packet.append(current_packet & 0xFF);
                packet.append(255U - (current_packet & 0xFF));
                //Payload
                const uint8_t pad_byte = 128U;
                uint16_t cksum = 0; //Same for CRC and checksum
                for(int i = 0; i<128; i++){
                    packet.append(pad_byte);
                    if(use_crc){
                        cksum = crc_update(cksum, &pad_byte, 1);
                    }
                    else{
                        cksum += pad_byte;
                    }
                }
                //Checksum
                if(use_crc){
                    packet.append((cksum >> 8) & 0xFF);
                    packet.append(cksum & 0xFF);
                }
                else{
                    packet.append(cksum & 0xFF);
                }

                //Signal we sent the packet
                pkcsPacketSent = true;
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

                    //If we just sent the PKCS packet, set the sendPkcsPacket
                    //flag to false so next "packet" is the end of transfer.
                    if(pkcsPacketSent){
                        sendPkcsPacket = false;
                    }
                }
                else{
                    //Seek only needed within file packets. PKCS padding needs
                    //no call to seek()
                    if(!pkcsPacketSent){
                        in_file->seek((current_packet - 1) * 128);
                    }
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
