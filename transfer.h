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

private:
    QSerialPort *serialPort;
    QString filePath;

signals:
    void updateProgress(float);
    void transferCompleted();
    void transferFailed();
};

#endif // TRANSFER_H
