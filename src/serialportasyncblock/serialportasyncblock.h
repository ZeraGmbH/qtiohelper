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
    QSerialPortAsyncBlock(QObject *parent = nullptr);
    void sendAndReceive(QByteArray dataSend, QByteArray* pDataReceive);
    void setReadTimeout(int iMsReceiveFirst, int iMsBetweenTwoBytes, int iMsMinTotal = 0);
    void setBlockEndCriteria(int iBlockLenReceive = 0, QByteArray endBlock = QByteArray());
    void enableDebugMessages(bool bEnable);
    bool isIOPending();
signals:
    void ioFinished();

public slots:

private slots:
    void onTimeoutFirst();
    void onTimeoutBetweenBytes();
    void onTimeoutMinTotal();
    void onReadyRead();
    void onError(SerialPortError serialError);

private:
    void onTimeoutCommon();
    QSerialPortAsyncBlockPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QSerialPortAsyncBlock)
};

#endif // QTSERIALPORTASYNCBLOCK_H
