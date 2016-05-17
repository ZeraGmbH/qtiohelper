#ifndef QTSERIALPORTASYNCBLOCK_P_H
#define QTSERIALPORTASYNCBLOCK_P_H

#include "serialportasyncblock.h"

class QSerialPortAsyncBlockPrivate
{
public:
    QSerialPortAsyncBlockPrivate();

    QByteArray m_dataSend;
    QByteArray* m_pDataReceive;

    int m_iMsFirstByte;
    int m_iMsBetweenTwoBytes;

    QByteArray m_endBlock;
    int m_iBlockLenReceive;

};
#endif // QTSERIALPORTASYNCBLOCK_P_H

