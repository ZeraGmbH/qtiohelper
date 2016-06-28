#include <QTimer>
#include "serialportasyncblock.h"
#include "serialportasyncblock_p.h"

// ************************** QSerialPortAsyncBlockPrivate

QSerialPortAsyncBlockPrivate::QSerialPortAsyncBlockPrivate()
{
    m_iMsReceiveFirst = 0;
    m_iMsBetweenTwoBytes = 0;
    m_iBlockLenReceive = 0;
    m_bIoPending = false;
}

// ************************** QSerialPortAsyncBlock
QSerialPortAsyncBlock::QSerialPortAsyncBlock(QObject *parent) :
    QSerialPort(parent),
    d_ptr(new QSerialPortAsyncBlockPrivate())
{
    Q_D(QSerialPortAsyncBlock);
    connect(&d->m_TimerForFirst, &QTimer::timeout, this, &QSerialPortAsyncBlock::onTimeout);
    connect(&d->m_TimerForBetweenTwoBytes, &QTimer::timeout, this, &QSerialPortAsyncBlock::onTimeout);
    connect(this, &QSerialPortAsyncBlock::readyRead, this, &QSerialPortAsyncBlock::onReadyRead);

    d->m_TimerForFirst.setSingleShot(true);
    d->m_TimerForBetweenTwoBytes.setSingleShot(true);
}

void QSerialPortAsyncBlock::sendAndReceive(QByteArray dataSend, QByteArray* pDataReceive)
{
    Q_D(QSerialPortAsyncBlock);
    d->m_dataSend = dataSend;
    // avoid data cumulation
    pDataReceive->resize(0);
    d->m_pDataReceive = pDataReceive;
    if(d->m_iMsReceiveFirst > 0)
        d->m_TimerForFirst.start(d->m_iMsReceiveFirst);
    clear(AllDirections);
    // empty send: read only
    if(dataSend.size() > 0)
        write(dataSend);
    d->m_bIoPending = true;
}

void QSerialPortAsyncBlock::setReadTimeout(int iMsReceiveFirst, int iMsBetweenTwoBytes)
{
    Q_D(QSerialPortAsyncBlock);
    d->m_iMsReceiveFirst = iMsReceiveFirst;
    d->m_iMsBetweenTwoBytes = iMsBetweenTwoBytes;
}

void QSerialPortAsyncBlock::setBlockEndCriteria(int iBlockLenReceive, QByteArray endBlock)
{
    Q_D(QSerialPortAsyncBlock);
    d->m_iBlockLenReceive = iBlockLenReceive;
    d->m_endBlock = endBlock;
}

bool QSerialPortAsyncBlock::isIOPending()
{
    Q_D(QSerialPortAsyncBlock);
    return d->m_bIoPending;
}

void QSerialPortAsyncBlock::onTimeout()
{
    Q_D(QSerialPortAsyncBlock);
    d->m_pDataReceive->append(readAll());
    clear(AllDirections);
    d->m_TimerForFirst.stop();
    d->m_TimerForBetweenTwoBytes.stop();
    d->m_bIoPending = false;
    emit ioFinished();
}

void QSerialPortAsyncBlock::onReadyRead()
{
    Q_D(QSerialPortAsyncBlock);
    QByteArray subBlock = readAll();
    // ignore received data after timeout
    if( (d->m_iMsReceiveFirst > 0 && d->m_TimerForFirst.isActive()) ||
        (d->m_iMsBetweenTwoBytes > 0 && d->m_TimerForBetweenTwoBytes.isActive()))
    {
        d->m_pDataReceive->append(subBlock);
        // we received data: stop timeout for first
        if(d->m_iMsBetweenTwoBytes > 0)
            d->m_TimerForFirst.stop();

        bool bFinish = false;
        // finish for blocklen?
        if(d->m_iBlockLenReceive > 0 && d->m_pDataReceive->count() >= d->m_iBlockLenReceive)
            bFinish = true;
        // finish for blockend
        if(d->m_endBlock.count() && d->m_pDataReceive->contains(d->m_endBlock))
            bFinish = true;
        // all work done?
        if(bFinish)
        {
            d->m_TimerForFirst.stop();
            d->m_TimerForBetweenTwoBytes.stop();
            d->m_bIoPending = false;
            emit ioFinished();
        }
        // check further with inter char timeout
        else if(d->m_iMsBetweenTwoBytes)
            d->m_TimerForBetweenTwoBytes.start(d->m_iMsBetweenTwoBytes);
    }
}
