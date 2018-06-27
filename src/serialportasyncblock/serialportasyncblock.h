#ifndef QTSERIALPORTASYNCBLOCK_H
#define QTSERIALPORTASYNCBLOCK_H

#include <QSerialPort>
#include <QByteArray>
#include "serialportasyncblock_global.h"

class QSerialPortAsyncBlockPrivate;

class QTSERIALPORTASYNCBLOCK_EXPORT QSerialPortAsyncBlock : public QSerialPort
{
    Q_OBJECT
public:
    QSerialPortAsyncBlock(QObject *parent = Q_NULLPTR);
    void sendAndReceive(QByteArray dataSend, QByteArray* pDataReceive);
    void setReadTimeout(int iMsReceiveFirst, int iMsBetweenTwoBytes);
    void setBlockEndCriteria(int iBlockLenReceive = 0, QByteArray endBlock = QByteArray());
    void enableDebugMessages(bool bEnable);
    bool isIOPending();
signals:
    void ioFinished();

public slots:

private slots:
    void onTimeout();
    void onReadyRead();
    void onError(SerialPortError serialError);

private:
    QSerialPortAsyncBlockPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QSerialPortAsyncBlock)
};

#endif // QTSERIALPORTASYNCBLOCK_H
