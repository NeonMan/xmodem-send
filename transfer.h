#ifndef TRANSFER_H
#define TRANSFER_H

#include <QObject>
#include <QSerialPort>
#include <QThread>

class Transfer : public QThread
{
    Q_OBJECT
public:
    explicit Transfer(
            QString serialPortName,
            qint32 baudrate,
            QString filePath,
            QObject *parent = nullptr
            );
    virtual ~Transfer();
    void launch();
    void cancel();

protected:
    //QThread method
    void run () override;

    static const quint32 timeoutRead      =  5000;
    static const quint32 timeoutFirstRead = 60000;

private:
    QSerialPort *serialPort;
    QString filePath;
    bool cancelRequested;

signals:
    void updateProgress(float);
    void transferCompleted();
    void transferFailed(QString);
};

#endif // TRANSFER_H
