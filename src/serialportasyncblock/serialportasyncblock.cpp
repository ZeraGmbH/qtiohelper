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
    m_bEnableDebugMessages = false;
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
#if QT_VERSION >= 0x050800
    connect(this, &QSerialPortAsyncBlock::errorOccurred, this, &QSerialPortAsyncBlock::onError);
#else
    connect(this, static_cast<void(QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error), this, &QSerialPortAsyncBlock::onError);
#endif

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
    d->m_bIoPending = clear(AllDirections);
    if(d->m_bIoPending)
    {
        // empty send: read only
        if(dataSend.size() > 0)
        {
            if(write(dataSend) <= 0)
                d->m_bIoPending = false;
        }
        if(d->m_bIoPending && d->m_iMsReceiveFirst > 0)
            d->m_TimerForFirst.start(d->m_iMsReceiveFirst);
    }
    // if nothing is started: end up here. This cannot be done by timeout
    // due to missing call of timeout slot
    if(!d->m_bIoPending)
        emit ioFinished();
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

void QSerialPortAsyncBlock::enableDebugMessages(bool bEnable)
{
    Q_D(QSerialPortAsyncBlock);
    d->m_bEnableDebugMessages = bEnable;
}

bool QSerialPortAsyncBlock::isIOPending()
{
    Q_D(QSerialPortAsyncBlock);
    return d->m_bIoPending;
}

void QSerialPortAsyncBlock::onTimeout()
{
    Q_D(QSerialPortAsyncBlock);
    if(d->m_bEnableDebugMessages)
        qInfo("Handle onTimeout");
    d->m_pDataReceive->append(readAll());
    clear(AllDirections);
    d->m_TimerForFirst.stop();
    d->m_TimerForBetweenTwoBytes.stop();
    d->m_bIoPending = false;
    emit ioFinished();
}

void QSerialPortAsyncBlock::onError(QSerialPort::SerialPortError serialError)
{
    Q_UNUSED(serialError);
    Q_D(QSerialPortAsyncBlock);
    if(d->m_bEnableDebugMessages)
        qWarning("Handle onError(\"%s\"", qPrintable(errorString()));
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
    if( d->m_bIoPending )
    {
        if(d->m_bEnableDebugMessages)
            qInfo("onReadyRead handled");
        d->m_pDataReceive->append(subBlock);
        // we received data: stop timeout for first
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
            d->m_TimerForBetweenTwoBytes.stop();
            d->m_bIoPending = false;
            emit ioFinished();
        }
        // check further with inter char timeout
        else if(d->m_iMsBetweenTwoBytes)
            d->m_TimerForBetweenTwoBytes.start(d->m_iMsBetweenTwoBytes);
    }
    else
    {
        if(d->m_bEnableDebugMessages)
            qInfo("onReadyRead not handled");
    }
}
