#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFormLayout>
#include <QComboBox>
#include <QProgressBar>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QPushButton>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    QComboBox * comboSerialPort;
    QComboBox * comboBaudrate;
    QLineEdit * editFilePath;
    QPushButton * pushBrowse;
    QProgressBar * progressFile;
    QPushButton * pushCancel;
    QPushButton * pushTransfer;

    void populate_widgets();

private slots:
    void onBrowseClicked();
    void onCancelClicked();
    void onTransferClicked();

    void updateProgress(float progress); //Values [0.0 1.0]
    void transferCompleted();
    void transferFailed();
};

#endif // MAINWINDOW_H
