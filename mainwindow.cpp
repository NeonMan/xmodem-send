#include "mainwindow.h"
#include "app-info.h"

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFileDialog>
#include <QProgressBar>
#include <QStatusBar>
#include <QSettings>
#include <QMessageBox>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //Set default attributes
    this->transferInstance = nullptr;

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
                this->pushCancel->setEnabled(false);
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

    //Status bar
    if(false) //<-- Disabled, for now.
    {
        QStatusBar *statusBar = new QStatusBar(this);
        statusBar->showMessage(tr("Ready"));
        this->statusBar = statusBar;
        this->setStatusBar(statusBar);
    }
    this->populate_widgets();

    //Connect QStettings-relevant widgets to onStoreSettings
    {
        connect(this->comboSerialPort, SIGNAL(currentIndexChanged(int)), this, SLOT(onStoreSettings(void)));
        connect(this->comboBaudrate, SIGNAL(currentIndexChanged(int)), this, SLOT(onStoreSettings(void)));
    }
}

MainWindow::~MainWindow()
{

}

void MainWindow::populate_widgets(){
    //Get previous values from QSettings
    QString lastPort;
    qint32  lastRate;
    {
        QSettings settings;
        lastRate = settings.value(KEY_LAST_BAUDRATE, 9600).toInt();
        lastPort = settings.value(KEY_LAST_PORT, "").toString();
    }

    //Populate com ports
    for(QSerialPortInfo i : QSerialPortInfo::availablePorts()){
        comboSerialPort->addItem(i.portName() + "  --  " + i.manufacturer(), QVariant(i.portName()));
        //Select last session's port, if available
        if(i.portName() == lastPort){
            comboSerialPort->setCurrentIndex(comboSerialPort->count() - 1);
        }
    }

    //Populate baudrates
    for(qint32 rate : QSerialPortInfo::standardBaudRates()){
        comboBaudrate->addItem(QString::number(rate), QVariant(rate));
        //Select last session's rate
        if(rate == lastRate){
            comboBaudrate->setCurrentIndex(comboBaudrate->count() - 1);
        }
    }
    //Make progress bar full
    progressFile->setValue(100);

    //Call onStoreSettings() to save the current status
    onStoreSettings();
}

void MainWindow::set_enabled_widgets(bool enable){
    this->pushBrowse->setEnabled(enable);
    this->pushTransfer->setEnabled(enable);
    this->editFilePath->setEnabled(enable);
    this->comboBaudrate->setEnabled(enable);
    this->comboSerialPort->setEnabled(enable);

    //Cancel button goes against the rest of the bunch
    this->pushCancel->setEnabled(!enable);
}

void MainWindow::onBrowseClicked(){
    qDebug() << __FILE__ << __LINE__ << "--" << __func__;
    QString path = QFileDialog::getOpenFileName(this, tr("Select file"));
    if(path.length() > 0){
        editFilePath->setText(path);
    }
}

void MainWindow::onCancelClicked(){
    qDebug() << __FILE__ << __LINE__ << "--" << __func__;
    this->transferInstance->cancel();
    set_enabled_widgets(true);
}

void MainWindow::onTransferClicked(){
    qDebug() << __FILE__ << __LINE__ << "--" << __func__;
    //If instance already exists, delete it
    if(this->transferInstance){
        delete this->transferInstance;
    }

    //Create a Transfer instance
    this->transferInstance = new Transfer(
                this->comboSerialPort->currentData().toString(),
                this->comboBaudrate->currentData().toInt(),
                this->editFilePath->text(),
                this);

    //Connect to signals
    connect(this->transferInstance, SIGNAL(updateProgress(float)), this, SLOT(updateProgress(float)));
    connect(this->transferInstance, SIGNAL(transferCompleted(void)), this, SLOT(onTransferCompleted(void)));
    connect(this->transferInstance, SIGNAL(transferFailed(QString)), this, SLOT(onTransferFailed(QString)));

    //Disable UI elements
    set_enabled_widgets(false);
    this->transferInstance->launch();
}

void MainWindow::updateProgress(float progress){
    {
        char progress_message[128];
        memset(progress_message, '\0', sizeof(progress_message));
        sprintf(progress_message, "(%f%%)", progress * 100);
        qDebug() << __FILE__ << __LINE__ << "--" << __func__ << progress_message;
    }
    progress = (progress > 1.0) ? 1.0 : progress;
    progress = (progress < 0.0) ? 0.0 : progress;
    this->progressFile->setValue(100.0 * progress);
}

void MainWindow::onTransferCompleted(){
    qDebug() << __FILE__ << __LINE__ << "--" << __func__;
    QMessageBox::information(this, tr("Transfer completed"), tr("Transfer completed"));
    set_enabled_widgets(true);
}

void MainWindow::onTransferFailed(QString reason){
    qDebug() << __FILE__ << __LINE__ << "--" << __func__ << "(" << reason << ")";
    QMessageBox::critical(this, tr("Transfer failed"), reason);
    set_enabled_widgets(true);

}

//Save relevant widget values into QSettings
void MainWindow::onStoreSettings(){
    qDebug() << __FILE__ << __LINE__ << "--" << __func__;
    QSettings settings;
    settings.setValue(KEY_LAST_BAUDRATE, this->comboBaudrate->currentData().toInt());
    settings.setValue(KEY_LAST_PORT, this->comboSerialPort->currentData().toString());
}

#include "moc_mainwindow.cpp"
