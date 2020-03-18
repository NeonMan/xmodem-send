#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFormLayout>
#include <QComboBox>
#include <QProgressBar>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QPushButton>

#include "transfer.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    //Widgets
    QComboBox * comboSerialPort;
    QComboBox * comboBaudrate;
    QLineEdit * editFilePath;
    QPushButton * pushBrowse;
    QProgressBar * progressFile;
    QPushButton * pushCancel;
    QPushButton * pushTransfer;
    QStatusBar * statusBar;

    //Other attributes
    Transfer * transferInstance;

    void populate_widgets();
    void set_enabled_widgets(bool enable = true);

private slots:
    void onStoreSettings();
    void onBrowseClicked();
    void onCancelClicked();
    void onTransferClicked();

    void updateProgress(float progress); //Values [0.0 1.0]
    void onTransferCompleted();
    void onTransferFailed();
};

#endif // MAINWINDOW_H
