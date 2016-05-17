#include <QTimer>
#include "serialportasyncblock.h"
#include "serialportasyncblock_p.h"

// ************************** QSerialPortAsyncBlockPrivate

QSerialPortAsyncBlockPrivate::QSerialPortAsyncBlockPrivate()
{
    m_iMsReceiveTotal = 0;
    m_iMsBetweenTwoBytes = 0;
    m_iBlockLenReceive = 0;
}

// ************************** QSerialPortAsyncBlock
QSerialPortAsyncBlock::QSerialPortAsyncBlock(QObject *parent) :
    QSerialPort(parent),
    d_ptr(new QSerialPortAsyncBlockPrivate())
{
    Q_D(QSerialPortAsyncBlock);
    connect(&d->m_TimeoutTimerTotal, &QTimer::timeout, this, &QSerialPortAsyncBlock::onTimeout);
    connect(&d->m_TimeoutTimerBlock, &QTimer::timeout, this, &QSerialPortAsyncBlock::onTimeout);
    connect(this, &QSerialPortAsyncBlock::readyRead, this, &QSerialPortAsyncBlock::onReadyRead);

    d->m_TimeoutTimerTotal.setSingleShot(true);
    d->m_TimeoutTimerBlock.setSingleShot(true);
}

void QSerialPortAsyncBlock::sendAndReceive(QByteArray dataSend, QByteArray* pDataReceive)
{
    Q_D(QSerialPortAsyncBlock);
    d->m_dataSend = dataSend;
    d->m_pDataReceive = pDataReceive;
    if(d->m_iMsReceiveTotal > 0)
        d->m_TimeoutTimerTotal.start(d->m_iMsReceiveTotal);
    clear(AllDirections);
    write(dataSend);
}

void QSerialPortAsyncBlock::setReadTimeout(int iMsReceiveTotal, int iMsBetweenTwoBytes)
{
    Q_D(QSerialPortAsyncBlock);
    d->m_iMsReceiveTotal = iMsReceiveTotal;
    d->m_iMsBetweenTwoBytes = iMsBetweenTwoBytes;
}

void QSerialPortAsyncBlock::setBlockLenReceive(int iBlockLenReceive)
{
    Q_D(QSerialPortAsyncBlock);
    d->m_iBlockLenReceive = iBlockLenReceive;
}

void QSerialPortAsyncBlock::setBlockEnd(QByteArray endBlock)
{
    Q_D(QSerialPortAsyncBlock);
    d->m_endBlock = endBlock;
}

void QSerialPortAsyncBlock::onTimeout()
{
    Q_D(QSerialPortAsyncBlock);
    d->m_pDataReceive->append(readAll());
    clear(AllDirections);
    d->m_TimeoutTimerTotal.stop();
    d->m_TimeoutTimerBlock.stop();
    emit IoFinished();
}

void QSerialPortAsyncBlock::onReadyRead()
{
    Q_D(QSerialPortAsyncBlock);
    d->m_pDataReceive->append(readAll());
    bool bFinish = false;
    // finish for blocklen?
    if(d->m_iBlockLenReceive > 0 && d->m_pDataReceive->count() >= d->m_iBlockLenReceive)
        bFinish = true;
    // finish for blockend
    if(d->m_endBlock.count() && d->m_pDataReceive->contains(d->m_endBlock))
        bFinish = true;
    if(bFinish)
    {
        d->m_TimeoutTimerTotal.stop();
        d->m_TimeoutTimerBlock.stop();
    }
    else if(d->m_iMsBetweenTwoBytes)
        d->m_TimeoutTimerBlock.start(d->m_iMsBetweenTwoBytes);
    if(bFinish)
        emit IoFinished();
}
