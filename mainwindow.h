/* Copyright (C) 2020 J.Luis <root@heavydeck.net>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFormLayout>
#include <QComboBox>
#include <QProgressBar>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>

#include "transfer.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    //Transfer
    QComboBox * comboSerialPort;
    QComboBox * comboBaudrate;
    QLineEdit * editFilePath;
    QPushButton * pushBrowse;
    QProgressBar * progressFile;
    QPushButton * pushCancel;
    QPushButton * pushTransfer;
    QStatusBar * statusBar;

    //Settings
    QWidget * widgetSettings;
    QComboBox * comboLanguage;
    QComboBox * comboParity;
    QComboBox * comboStopBits;
    QComboBox * comboFlowControl;
    QCheckBox * checkUsePkcsPadding;

    //Other attributes
    Transfer * transferInstance;

    void populate_widgets();
    void set_enabled_widgets(bool enable = true);

private slots:
    void onStoreSettings();
    void onLanguageChanged();
    void onBrowseClicked();
    void onCancelClicked();
    void onTransferClicked();

    void updateProgress(float progress); //Values [0.0 1.0]
    void onTransferCompleted();
    void onTransferFailed(QString reason);
};

#endif // MAINWINDOW_H
