#include "bit-input-poller.h"
#include "bit-input-poller_p.h"

// ************************** QBitInputPollerPrivate

QBitInputPollerPrivate::QBitInputPollerPrivate()
{
    m_pStartBitReadFunction = nullptr;
}

QBitInputPollerPrivate::~QBitInputPollerPrivate()
{
}

// ************************** QBitInputPoller

QBitInputPoller::QBitInputPoller(QObject *parent) :
    QObject(parent),
    d_ptr(new QBitInputPollerPrivate())
{
    Q_D(QBitInputPoller);
    connect(&d->m_PollTimer, &QTimer::timeout, this, &QBitInputPoller::onPollTimer);
}

void QBitInputPoller::setupInputMask(int iCountBits, const QBitArray &mask)
{
    Q_D(QBitInputPoller);
    d->m_BitMaskInput.resize(iCountBits);
    d->m_BitMaskInvert = mask;
    if(!d->m_BitMaskInvert.isEmpty()) {
        d->m_BitMaskInvert.resize(iCountBits);
    }
}

const QBitArray* QBitInputPoller::getInputBitmask()
{
    Q_D(QBitInputPoller);
    return &d->m_BitMaskInput;
}

void QBitInputPoller::setStartBitReadFunction(StartBitReadFunction pFunc)
{
    Q_D(QBitInputPoller);
    d->m_pStartBitReadFunction = pFunc;
}


void QBitInputPoller::startPoll(int iMilliSecCycle)
{
    Q_D(QBitInputPoller);
    d->m_PollTimer.setSingleShot(false);
    d->m_PollTimer.start(iMilliSecCycle);
}

void QBitInputPoller::stopPoll()
{
    Q_D(QBitInputPoller);
    d->m_PollTimer.stop();
}


void QBitInputPoller::onPollTimer()
{
    Q_D(QBitInputPoller);
    if(d->m_pStartBitReadFunction) {
        bool bBitHandlerBusy = d->m_pStartBitReadFunction(&d->m_BitMaskInput);
        if(!bBitHandlerBusy) {
            if(!d->m_BitMaskInvert.isEmpty()) {
                d->m_BitMaskInput ^= d->m_BitMaskInvert;
            }
            emit bitmaskUpdated();
        }
    }
}

void QBitInputPoller::onBitMaskReadFinish()
{
    Q_D(QBitInputPoller);
    if(!d->m_BitMaskInvert.isEmpty()) {
        d->m_BitMaskInput ^= d->m_BitMaskInvert;
    }
    // for now just pass over
    emit bitmaskUpdated();
}
