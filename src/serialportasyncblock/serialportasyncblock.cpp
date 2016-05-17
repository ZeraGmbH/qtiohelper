#include "serialportasyncblock.h"
#include "serialportasyncblock_p.h"

// ************************** QSerialPortAsyncBlockPrivate

QSerialPortAsyncBlockPrivate::QSerialPortAsyncBlockPrivate()
{
    m_iMsFirstByte = 0;
    m_iMsBetweenTwoBytes = 0;
    m_iBlockLenReceive = 0;
}

// ************************** QSerialPortAsyncBlock
QSerialPortAsyncBlock::QSerialPortAsyncBlock(QObject *parent) :
    QSerialPort(parent),
    d_ptr(new QSerialPortAsyncBlockPrivate())
{
    connect(&m_timeoutTimer, &QTimer::timeout, this, &QSerialPortAsyncBlock::onTimeout);
    connect(this, &QSerialPortAsyncBlock::readyRead, this, &QSerialPortAsyncBlock::onReadyRead);

    m_timeoutTimer.setSingleShot(true);
}

void QSerialPortAsyncBlock::sendAndReceive(QByteArray dataSend, QByteArray* pDataReceive)
{
    Q_D(QSerialPortAsyncBlock);
    d->m_dataSend = dataSend;
    d->m_pDataReceive = pDataReceive;
    if(d->m_iMsFirstByte > 0)
        m_timeoutTimer.start(d->m_iMsFirstByte);
    clear(AllDirections);
    write(dataSend);
}

void QSerialPortAsyncBlock::setReadTimeout(int iMsFirstByte, int iMsBetweenTwoBytes)
{
    Q_D(QSerialPortAsyncBlock);
    d->m_iMsFirstByte = iMsFirstByte;
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
    if(d->m_iMsFirstByte)
        m_timeoutTimer.stop();
    if(d->m_iMsBetweenTwoBytes)
        m_timeoutTimer.start(d->m_iMsBetweenTwoBytes);
    if(bFinish)
        emit IoFinished();
}
