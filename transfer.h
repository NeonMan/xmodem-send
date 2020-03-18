#ifndef TRANSFER_H
#define TRANSFER_H

#include <QObject>
#include <QSerialPort>

class Transfer : public QObject
{
    Q_OBJECT
public:
    explicit Transfer(
            QString serialPortName,
            qint32 baudrate,
            QString filePath,
            QObject *parent = nullptr
            );
    ~Transfer();
    void launch();
    void cancel();

private:
    QSerialPort *serialPort;
    QString filePath;

signals:
    void updateProgress(float);
    void transferCompleted();
    void transferFailed();
};

#endif // TRANSFER_H
