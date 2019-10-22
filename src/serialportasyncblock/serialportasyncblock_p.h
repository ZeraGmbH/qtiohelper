#ifndef QTSERIALPORTASYNCBLOCK_P_H
#define QTSERIALPORTASYNCBLOCK_P_H

#include "serialportasyncblock.h"
#include <QTimer>
#include <QString>

class QSerialPortAsyncBlockPrivate
{
public:
    QSerialPortAsyncBlockPrivate();
    void outputDebug(const QString strMsg, bool bWarning = false);

    QByteArray m_dataSend;
    QByteArray* m_pDataReceive;

    int m_iMsReceiveFirst;
    int m_iMsBetweenTwoBytes;
    int m_iMsMinTotal;

    QByteArray m_endBlock;
    int m_iBlockLenReceive;

    QTimer m_TimerForFirst;
    QTimer m_TimerForBetweenTwoBytes;
    QTimer m_TimerForMinTotal;

    bool m_bIoPending;
    bool m_bEnableDebugMessages;
};
#endif // QTSERIALPORTASYNCBLOCK_P_H

