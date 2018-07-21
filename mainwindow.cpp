#include "mainwindow.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //Build the UI widgets.
    {
        QWidget * widgetCentral = new QWidget(this);
        this->setCentralWidget(widgetCentral);
        QFormLayout* mainLayout = new QFormLayout(widgetCentral);
        {
            QComboBox * comboSerialPort = new QComboBox(widgetCentral);
            this->comboSerialPort = comboSerialPort;
            QComboBox * comboBaudrate = new QComboBox(widgetCentral);
            this->comboBaudrate = comboBaudrate;
            QWidget * widgetFilePick = new QWidget(widgetCentral);
            {
                QHBoxLayout * layoutFilePick = new QHBoxLayout(widgetFilePick);
                layoutFilePick->setMargin(0);

                QLineEdit * editFilePath = new QLineEdit(widgetFilePick);
                this->editFilePath = editFilePath;
                QPushButton * pushBrowse = new QPushButton(tr("Browse"), widgetFilePick);
                this->pushBrowse = pushBrowse;

                connect(pushBrowse, SIGNAL(clicked(bool)), this, SLOT(onBrowseClicked()));

                layoutFilePick->addWidget(editFilePath);
                layoutFilePick->addWidget(pushBrowse);
            }
            QProgressBar * progressFile = new QProgressBar(widgetCentral);
            progressFile->setMaximum(100);
            progressFile->setMinimum(0);
            this->progressFile = progressFile;
            QWidget * widgetButtons = new QWidget(widgetCentral);
            {
                QHBoxLayout * layoutButtons = new QHBoxLayout(widgetButtons);
                layoutButtons->setMargin(0);
                layoutButtons->addStretch(1000);

                QPushButton * pushCancel = new QPushButton(tr("Cancel"), widgetButtons);
                this->pushCancel = pushCancel;
                QPushButton * pushTransfer = new QPushButton(tr("Transfer"), widgetButtons);
                this->pushTransfer = pushTransfer;

                connect(pushCancel, SIGNAL(clicked(bool)), this, SLOT(onCancelClicked()));
                connect(pushTransfer, SIGNAL(clicked(bool)), this, SLOT(onTransferClicked()));

                layoutButtons->addWidget(pushCancel);
                layoutButtons->addWidget(pushTransfer);
            }
            mainLayout->addRow(tr("Port"), comboSerialPort);
            mainLayout->addRow(tr("Baudrate"), comboBaudrate);
            mainLayout->addRow(tr("File"), widgetFilePick);
            mainLayout->addRow(tr(""), progressFile);
            mainLayout->addWidget(widgetButtons);
        }
    }

    populate_widgets();
}

MainWindow::~MainWindow()
{

}

void MainWindow::populate_widgets(){
    //Populate com ports
    for(QSerialPortInfo i : QSerialPortInfo::availablePorts()){
        comboSerialPort->addItem(i.portName(), QVariant(i.portName()));
    }

    //Populate baudrates
    for(qint32 rate : QSerialPortInfo::standardBaudRates()){
        comboBaudrate->addItem(QString::number(rate), QVariant(rate));
        if(rate == 9600){
            comboBaudrate->setCurrentIndex(comboBaudrate->count() - 1);
        }
    }
    //Make progress bar full
    progressFile->setValue(0);
}

void MainWindow::onBrowseClicked(){
    QString path = QFileDialog::getOpenFileName(this, tr("Select file"));
    if(path.length() > 0){
        editFilePath->setText(path);
    }
}

void MainWindow::onCancelClicked(){

}

void MainWindow::onTransferClicked(){

}

void MainWindow::updateProgress(float progress){
    progress = (progress > 1.0) ? 1.0 : progress;
    progress = (progress < 0.0) ? 0.0 : progress;
    this->progressFile->setValue(100 * progress);
}

void MainWindow::transferCompleted(){

}

void MainWindow::transferFailed(){

}

#include "moc_mainwindow.cpp"
