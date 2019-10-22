#include "serialportasyncblock.h"
#include "serialportasyncblock_p.h"
#include <QTimer>
#include <QDateTime>
#include <QDebug>

// ************************** QSerialPortAsyncBlockPrivate

QSerialPortAsyncBlockPrivate::QSerialPortAsyncBlockPrivate()
{
    m_iMsReceiveFirst = 0;
    m_iMsBetweenTwoBytes = 0;
    m_iBlockLenReceive = 0;
    m_bIoPending = false;
    m_bEnableDebugMessages = false;
}

void QSerialPortAsyncBlockPrivate::outputDebug(const QString strMsg, bool bWarning)
{
    if(m_bEnableDebugMessages) {
        QString strOut = QString(QStringLiteral("[%1]: %2")).arg(QDateTime::currentDateTime().toString(QStringLiteral("hh:mm:ss.zzz"))).arg(strMsg);
        if(bWarning) {
            qWarning("%s", qPrintable(strOut));
        }
        else {
            qInfo("%s", qPrintable(strOut));
        }
    }
}

// ************************** QSerialPortAsyncBlock
QSerialPortAsyncBlock::QSerialPortAsyncBlock(QObject *parent) :
    QSerialPort(parent),
    d_ptr(new QSerialPortAsyncBlockPrivate())
{
    Q_D(QSerialPortAsyncBlock);
    connect(&d->m_TimerForFirst, &QTimer::timeout, this, &QSerialPortAsyncBlock::onTimeoutFirst);
    connect(&d->m_TimerForBetweenTwoBytes, &QTimer::timeout, this, &QSerialPortAsyncBlock::onTimeoutBetweenBytes);
    connect(&d->m_TimerForMinTotal, &QTimer::timeout, this, &QSerialPortAsyncBlock::onTimeoutMinTotal);
    connect(this, &QSerialPortAsyncBlock::readyRead, this, &QSerialPortAsyncBlock::onReadyRead);
#if QT_VERSION >= 0x050800
    connect(this, &QSerialPortAsyncBlock::errorOccurred, this, &QSerialPortAsyncBlock::onError);
#else
    connect(this, static_cast<void(QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error), this, &QSerialPortAsyncBlock::onError);
#endif

    d->m_TimerForFirst.setSingleShot(true);
    d->m_TimerForBetweenTwoBytes.setSingleShot(true);
    d->m_TimerForMinTotal.setSingleShot(true);
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
        if(dataSend.size() > 0) {
            if(write(dataSend) <= 0) {
                d->m_bIoPending = false;
            }
        }
        if(d->m_bIoPending && d->m_iMsReceiveFirst > 0) {
            d->m_TimerForFirst.start(d->m_iMsReceiveFirst);
        }
        if(d->m_bIoPending && d->m_iMsMinTotal > 0) {
            d->m_TimerForMinTotal.start(d->m_iMsMinTotal);
        }
    }
    // if nothing is started: end up here. This cannot be done by timeout
    // due to missing call of timeout slot
    if(!d->m_bIoPending) {
        emit ioFinished();
    }
}

void QSerialPortAsyncBlock::setReadTimeout(int iMsReceiveFirst, int iMsBetweenTwoBytes, int iMsMinTotal)
{
    Q_D(QSerialPortAsyncBlock);
    d->m_iMsReceiveFirst = iMsReceiveFirst;
    d->m_iMsBetweenTwoBytes = iMsBetweenTwoBytes;
    d->m_iMsMinTotal = iMsMinTotal;
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

void QSerialPortAsyncBlock::onTimeoutFirst()
{
    Q_D(QSerialPortAsyncBlock);
    d->outputDebug(QStringLiteral("Handle onTimeoutFirst"));
    onTimeoutCommon();
}

void QSerialPortAsyncBlock::onTimeoutBetweenBytes()
{
    Q_D(QSerialPortAsyncBlock);
    d->outputDebug(QStringLiteral("Handle onTimeoutBetweenBytes"));
    onTimeoutCommon();
}

void QSerialPortAsyncBlock::onTimeoutCommon()
{
    Q_D(QSerialPortAsyncBlock);
    if(d->m_pDataReceive) {
        d->m_pDataReceive->append(readAll());
    }
    d->m_TimerForFirst.stop();
    d->m_TimerForBetweenTwoBytes.stop();
    if(!d->m_TimerForMinTotal.isActive()) {
        clear(AllDirections);
        if(d->m_bIoPending) {
            d->m_bIoPending = false;
            emit ioFinished();
        }
    }
}

void QSerialPortAsyncBlock::onTimeoutMinTotal()
{
    Q_D(QSerialPortAsyncBlock);
    d->outputDebug(QStringLiteral("Handle onTimeoutMinTotal"));
    // This is the weakest timeout and causing effect only if
    // the other timeouts are not active
    if(d->m_bIoPending && !d->m_TimerForFirst.isActive() && !d->m_TimerForBetweenTwoBytes.isActive()) {
        if(d->m_pDataReceive) {
            d->m_pDataReceive->append(readAll());
        }
        clear(AllDirections);
        d->m_bIoPending = false;
        emit ioFinished();
    }
}

void QSerialPortAsyncBlock::onError(QSerialPort::SerialPortError serialError)
{
    Q_UNUSED(serialError)
    Q_D(QSerialPortAsyncBlock);
    d->outputDebug(QStringLiteral("Handle onError: ")+errorString(), true);
    d->m_TimerForFirst.stop();
    d->m_TimerForBetweenTwoBytes.stop();
    d->m_TimerForMinTotal.stop();
    if(d->m_bIoPending) {
        if(d->m_pDataReceive && isOpen())
            d->m_pDataReceive->append(readAll());
        d->m_bIoPending = false;
        emit ioFinished();
    }
}

void QSerialPortAsyncBlock::onReadyRead()
{
    Q_D(QSerialPortAsyncBlock);
    QByteArray subBlock = readAll();
    // ignore received data after timeout
    if( d->m_bIoPending ) {
        d->outputDebug(QStringLiteral("onReadyRead handled"));
        if(d->m_pDataReceive) {
            d->m_pDataReceive->append(subBlock);
        }
        // we received data: stop timeout for first
        d->m_TimerForFirst.stop();

        bool bFinish = false;
        // finish for blocklen?
        if(d->m_iBlockLenReceive > 0 && d->m_pDataReceive->count() >= d->m_iBlockLenReceive) {
            bFinish = true;
        }
        // finish for blockend
        if(d->m_endBlock.count() && d->m_pDataReceive && d->m_pDataReceive->contains(d->m_endBlock)) {
            bFinish = true;
        }
        // all work done?
        if(bFinish) {
            d->m_TimerForBetweenTwoBytes.stop();
            d->m_TimerForMinTotal.stop();
            d->m_bIoPending = false;
            emit ioFinished();
        }
        // check further with inter char timeout
        else if(d->m_iMsBetweenTwoBytes) {
            // singleshot timers do not restart
            if(d->m_TimerForBetweenTwoBytes.isActive())
                d->m_TimerForBetweenTwoBytes.stop();
            d->m_TimerForBetweenTwoBytes.start(d->m_iMsBetweenTwoBytes);
        }
    }
    else {
        d->outputDebug(QStringLiteral("onReadyRead not handled"));
    }
}
