#include "transfer.h"
#include <QSerialPortInfo>
#include <QDebug>

Transfer::Transfer(QString serialPortName, qint32 baudrate, QString filePath, QObject *parent)  : QThread(parent)
{
    qDebug() << __FILE__ << __LINE__ << "--" << __func__;
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
    this->run();
}

void Transfer::cancel(){
    qDebug() << __FILE__ << __LINE__ << "--" << __func__;
}


//The bulk of the work goes here
void Transfer::run(){
    qDebug() << __FILE__ << __LINE__ << "--" << __func__;

    //Reset progress
    emit updateProgress(0.0f);

    //Transfer completed
    emit updateProgress(1.0f);
}
