#ifndef QTSERIALPORTASYNCBLOCK_P_H
#define QTSERIALPORTASYNCBLOCK_P_H

#include <QTimer>
#include "serialportasyncblock.h"

class QSerialPortAsyncBlockPrivate
{
public:
    QSerialPortAsyncBlockPrivate();

    QByteArray m_dataSend;
    QByteArray* m_pDataReceive;

    int m_iMsReceiveTotal;
    int m_iMsBetweenTwoBytes;

    QByteArray m_endBlock;
    int m_iBlockLenReceive;

    QTimer m_TimeoutTimerTotal;
    QTimer m_TimeoutTimerBlock;
};
#endif // QTSERIALPORTASYNCBLOCK_P_H

