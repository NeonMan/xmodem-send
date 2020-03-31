/* Copyright (C) 2020 J.Luis <root@heavydeck.net>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */
#include "mainwindow.h"
#include "app-info.h"

#include <QFileDialog>
#include <QProgressBar>
#include <QStatusBar>
#include <QTabWidget>
#include <QLabel>


#include <QSettings>
#include <QMessageBox>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFileInfo>
#include <QDir>
#include <QLocale>

#include <QApplication>
#include <QDebug>

///This function inserts an element into the combobox and if data matches with indexData, sets current index to that element.
static void addComboElementAndSetIndex(QComboBox *comboBox, QString label, QVariant data, QVariant indexData = QVariant()){
    comboBox->addItem(label, data);
    if(data == indexData){
        comboBox->setCurrentIndex(comboBox->count() - 1);
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //Set default attributes
    this->transferInstance = nullptr;
    this->setWindowTitle(tr("XMODEM upload tool"));

    //Build the UI widgets.
    {
        QTabWidget * tabWidgetMain = new QTabWidget(this);
        this->setCentralWidget(tabWidgetMain);

        //Transfer tab
        {
            QWidget * widgetTransfer = new QWidget(tabWidgetMain);
            QFormLayout* transferLayout = new QFormLayout(widgetTransfer);
            {
                QComboBox * comboSerialPort = new QComboBox(widgetTransfer);
                this->comboSerialPort = comboSerialPort;
                QComboBox * comboBaudrate = new QComboBox(widgetTransfer);
                this->comboBaudrate = comboBaudrate;
                QWidget * widgetFilePick = new QWidget(widgetTransfer);
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
                QProgressBar * progressFile = new QProgressBar(widgetTransfer);
                progressFile->setMaximum(1000);
                progressFile->setMinimum(0);
                progressFile->setTextVisible(true);
                this->progressFile = progressFile;
                QWidget * widgetButtons = new QWidget(widgetTransfer);
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
                transferLayout->addRow(tr("Port"), comboSerialPort);
                transferLayout->addRow(tr("Baudrate"), comboBaudrate);
                transferLayout->addRow(tr("File"), widgetFilePick);
                transferLayout->addRow(tr(""), progressFile);
                transferLayout->addWidget(widgetButtons);
            }
            tabWidgetMain->addTab(widgetTransfer, tr("Transfer"));
        }

        //Settings tab
        {
            QWidget * widgetSettings = new QWidget(tabWidgetMain);
            QFormLayout* layoutSettings = new QFormLayout(widgetSettings);
            this->widgetSettings = widgetSettings;

            QComboBox * comboLanguage = new QComboBox(widgetSettings);
            this->comboLanguage = comboLanguage;

            QComboBox * comboParity = new QComboBox(widgetSettings);
            this->comboParity = comboParity;

            QComboBox * comboStopBits = new QComboBox(widgetSettings);
            this->comboStopBits = comboStopBits;

            QComboBox * comboFlowControl = new QComboBox(widgetSettings);
            this->comboFlowControl = comboFlowControl;

            QCheckBox * checkUsePkcsPadding = new QCheckBox(widgetSettings);
            this->checkUsePkcsPadding = checkUsePkcsPadding;

            layoutSettings->addRow(tr("Language"), comboLanguage);
            layoutSettings->addRow(tr("Parity"), comboParity);
            layoutSettings->addRow(tr("Stop bits"), comboStopBits);
            layoutSettings->addRow(tr("Flow control"), comboFlowControl);
            layoutSettings->addRow(tr("PKCS#7 padding"), checkUsePkcsPadding);

            tabWidgetMain->addTab(widgetSettings, tr("Settings"));
        }

        //About tab
        {
            QWidget * widgetAbout = new QWidget(tabWidgetMain);
            QVBoxLayout * layoutAbout = new QVBoxLayout(widgetAbout);
            //this->widgetAbout = widgetAbout;

            //Disclaimer
            QLabel* labelAboutText = new QLabel(widgetAbout);
            labelAboutText->setWordWrap(true);
            labelAboutText->setText(QApplication::applicationName() + " (C) 2020 heavydeck.net; " +
                        "This program is free software; you can redistribute it and/or "
                        "modify it under the terms of the GNU General Public License version 2."
                        );
            layoutAbout->addWidget(labelAboutText);

            //Qt
            QLabel* labelAboutQt = new QLabel(widgetAbout);
            labelAboutQt->setWordWrap(true);
            labelAboutQt->setText(
                        "The Qt framework is released under the GNU Lesser General Public License version 3."
                        );
            layoutAbout->addWidget(labelAboutQt);

            //Icon
            QLabel* labelAboutIcon = new QLabel(widgetAbout);
            labelAboutIcon->setWordWrap(true);
            labelAboutIcon->setText(
                        "Icon by 'Those Icons' at flaticon.com."
                        );
            layoutAbout->addWidget(labelAboutIcon);

            tabWidgetMain->addTab(widgetAbout, tr("About"));
        }
    }

    //Status bar
    {
        QStatusBar *statusBar = new QStatusBar(this);
        statusBar->showMessage(tr("Ready"));
        this->statusBar = statusBar;
        this->setStatusBar(statusBar);
        statusBar->setVisible(false); //<-- Disable, for now.
    }
    this->populate_widgets();

    //Connect QStettings-relevant widgets to onStoreSettings
    {
        connect(this->comboSerialPort, SIGNAL(currentIndexChanged(int)), this, SLOT(onStoreSettings(void)));
        connect(this->comboBaudrate, SIGNAL(currentIndexChanged(int)), this, SLOT(onStoreSettings(void)));
        connect(this->comboLanguage, SIGNAL(currentIndexChanged(int)), this, SLOT(onStoreSettings(void)));
        connect(this->comboParity, SIGNAL(currentIndexChanged(int)), this, SLOT(onStoreSettings(void)));
        connect(this->comboStopBits, SIGNAL(currentIndexChanged(int)), this, SLOT(onStoreSettings(void)));
        connect(this->comboFlowControl, SIGNAL(currentIndexChanged(int)), this, SLOT(onStoreSettings(void)));
        connect(this->checkUsePkcsPadding, SIGNAL(stateChanged(int)), this, SLOT(onStoreSettings(void)));
    }
    //Rest of slot connections
    {
        connect(this->comboLanguage, SIGNAL(currentIndexChanged(int)), this, SLOT(onLanguageChanged(void)));
    }
}

MainWindow::~MainWindow()
{

}

void MainWindow::populate_widgets(){
    //Get previous values from QSettings
    QString lastPort;
    QString lastLanguage;
    qint32  lastRate;
    int lastParity;
    int lastStopBits;
    int lastFlow;
    bool usePkcs;
    {
        QSettings settings;
        lastRate     = settings.value(KEY_LAST_BAUDRATE, 9600).toInt();
        lastPort     = settings.value(KEY_LAST_PORT, "").toString();
        lastParity   = settings.value(KEY_LAST_PARITY, (int)QSerialPort::Parity::NoParity).toInt();
        lastStopBits = settings.value(KEY_LAST_STOP_BITS, (int)QSerialPort::StopBits::OneStop).toInt();
        lastFlow     = settings.value(KEY_LAST_FLOW_CONTROL, (int)QSerialPort::FlowControl::NoFlowControl).toInt();
        usePkcs      = settings.value(KEY_USE_PKCS_PADDING, false).toBool();
        lastLanguage = settings.value(KEY_LANGUAGE, QLocale::system().name()).toString();
    }

    //Populate com ports
    for(QSerialPortInfo i : QSerialPortInfo::availablePorts()){
        addComboElementAndSetIndex(comboSerialPort, i.portName() + "  --  " + i.manufacturer(), i.portName(), lastPort);
    }

    //Populate baudrates
    for(qint32 rate : QSerialPortInfo::standardBaudRates()){
        addComboElementAndSetIndex(comboBaudrate, QString::number(rate), rate, lastRate);
    }
    //Make progress bar full
    progressFile->setValue(progressFile->maximum());

    //Language combo
    {
        //Note: Entries are in the format:
        //  * Language name on the language itself
        //  * Language name on the current tr() language, between parentheses

        //                                              Language name  + translated name, Locale name
        addComboElementAndSetIndex(this->comboLanguage, "English "     + tr("(English)"), "",      lastLanguage);
        addComboElementAndSetIndex(this->comboLanguage, "Español "     + tr("(Spanish)"), "es_ES", lastLanguage);
    }

    //Parity combo
    {
        addComboElementAndSetIndex(this->comboParity, tr("None"), (int)QSerialPort::Parity::NoParity, lastParity);
        addComboElementAndSetIndex(this->comboParity, tr("Odd"), (int)QSerialPort::Parity::OddParity, lastParity);
        addComboElementAndSetIndex(this->comboParity, tr("Even"), (int)QSerialPort::Parity::EvenParity, lastParity);
        addComboElementAndSetIndex(this->comboParity, tr("Mark"), (int)QSerialPort::Parity::MarkParity, lastParity);
        addComboElementAndSetIndex(this->comboParity, tr("Space"), (int)QSerialPort::Parity::SpaceParity, lastParity);
    }

    //Stop bit combo
    {
        addComboElementAndSetIndex(this->comboStopBits, "1", (int)QSerialPort::StopBits::OneStop, lastStopBits);
        addComboElementAndSetIndex(this->comboStopBits, "1.5", (int)QSerialPort::StopBits::OneAndHalfStop, lastStopBits);
        addComboElementAndSetIndex(this->comboStopBits, "2", (int)QSerialPort::StopBits::TwoStop, lastStopBits);
    }

    //Flow control
    {
        addComboElementAndSetIndex(this->comboFlowControl, tr("None"), (int)QSerialPort::FlowControl::NoFlowControl, lastFlow);
        addComboElementAndSetIndex(this->comboFlowControl, tr("Hardware"), (int)QSerialPort::FlowControl::HardwareControl, lastFlow);
    }

    //Padding
    {
        this->checkUsePkcsPadding->setChecked(usePkcs);
    }

    //Call onStoreSettings() to save the current status
    onStoreSettings();
}

void MainWindow::set_enabled_widgets(bool enable){
    this->pushBrowse->setEnabled(enable);
    this->pushTransfer->setEnabled(enable);
    this->editFilePath->setEnabled(enable);
    this->comboBaudrate->setEnabled(enable);
    this->comboSerialPort->setEnabled(enable);

    //The whole config is enabled/disabled
    widgetSettings->setEnabled(enable);

    //Cancel button goes against the rest of the bunch
    this->pushCancel->setEnabled(!enable);
}

void MainWindow::onBrowseClicked(){
    qDebug() << __FILE__ << __LINE__ << "--" << __func__;
    QSettings settings;
    QString last_directory;
    last_directory = settings.value(KEY_LAST_DIRECTORY, "").toString();

    QString path = QFileDialog::getOpenFileName(this, tr("Select file"), last_directory);
    if(path.length() > 0){
        editFilePath->setText(path);
        //Save last path to settings
        QFileInfo f_info (path);
        settings.setValue(KEY_LAST_DIRECTORY, f_info.dir().absolutePath());
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
    //Configure it
    this->transferInstance->setParity((QSerialPort::Parity)this->comboParity->currentData().toInt());
    this->transferInstance->setStopBits((QSerialPort::StopBits)this->comboStopBits->currentData().toInt());
    this->transferInstance->SetFlowControl((QSerialPort::FlowControl)this->comboFlowControl->currentData().toInt());
    this->transferInstance->setPkcsPadding(this->checkUsePkcsPadding->isChecked());

    //Connect to signals
    connect(this->transferInstance, SIGNAL(updateProgress(float)), this, SLOT(updateProgress(float)));
    connect(this->transferInstance, SIGNAL(transferCompleted(void)), this, SLOT(onTransferCompleted(void)));
    connect(this->transferInstance, SIGNAL(transferFailed(QString)), this, SLOT(onTransferFailed(QString)));

    //Disable UI elements
    set_enabled_widgets(false);

    //Launch
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
    this->progressFile->setValue(this->progressFile->maximum() * progress);
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

void MainWindow::onLanguageChanged(){
    qDebug() << __FILE__ << __LINE__ << "--" << __func__;
    QMessageBox::information(this, tr("Restart required"), tr("Application restart required for language change to be effective."));
}

//Save relevant widget values into QSettings
void MainWindow::onStoreSettings(){
    qDebug() << __FILE__ << __LINE__ << "--" << __func__;
    QSettings settings;
    settings.setValue(KEY_LAST_BAUDRATE, this->comboBaudrate->currentData().toInt());
    settings.setValue(KEY_LAST_PORT, this->comboSerialPort->currentData().toString());
    settings.setValue(KEY_LAST_PARITY, this->comboParity->currentData().toInt());
    settings.setValue(KEY_LAST_STOP_BITS, this->comboStopBits->currentData().toInt());
    settings.setValue(KEY_LAST_FLOW_CONTROL, this->comboFlowControl->currentData().toInt());
    settings.setValue(KEY_USE_PKCS_PADDING, this->checkUsePkcsPadding->isChecked());
    settings.setValue(KEY_LANGUAGE, this->comboLanguage->currentData().toString());
}

#include "moc_mainwindow.cpp"
