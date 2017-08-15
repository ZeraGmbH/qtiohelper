#include "relay-base.h"
#include "relay-base_p.h"

// ************************** QRelayBasePrivate

QRelayBasePrivate::QRelayBasePrivate()
{
}

QRelayBasePrivate::~QRelayBasePrivate()
{
}

// ************************** QRelayUpperBasePrivate
QRelayUpperBasePrivate::QRelayUpperBasePrivate()
{
    lowRelayLayer = nullptr;
}

// ************************** QRelayBase

QRelayBase::QRelayBase(QObject *parent, QRelayBasePrivate *dp) :
    QObject(parent),
    d_ptr(dp)
{
}

QRelayBase::~QRelayBase()
{
    delete d_ptr;
}

quint16 QRelayBase::getLogicalRelayCount()
{
    return getLogicalRelayState().size();
}

void QRelayBase::setupBaseBitmaps(quint16 ui16LogicalArrayInfoCount)
{
    Q_D(QRelayBase);
    d->logicalEnableMaskNext = QBitArray(ui16LogicalArrayInfoCount);
    d->logicalSetMaskNext = QBitArray(ui16LogicalArrayInfoCount);
    d->logicalBusyMask = QBitArray(ui16LogicalArrayInfoCount);
}

void QRelayBase::startSetMulti(const QBitArray& logicalEnableMask,
                               const QBitArray& logicalSetMask,
                               bool bForce)
{
    Q_D(QRelayBase);
    for(int iBit=0;
        iBit<logicalEnableMask.size() && iBit<getLogicalRelayCount();
        iBit++)
    {
        if(logicalEnableMask.at(iBit))
            startSet(iBit, iBit<logicalSetMask.size() ? logicalSetMask.at(iBit) : false, bForce);
    }
}

void QRelayBase::startSet(quint16 ui16BitNo,
                          bool bSet,
                          bool bForce)
{
    Q_D(QRelayBase);
    if(ui16BitNo < getLogicalRelayCount())
    {
        if(bForce || bSet != getLogicalRelayState().at(ui16BitNo))
        {
            d->logicalEnableMaskNext.setBit(ui16BitNo);
            d->logicalSetMaskNext.setBit(ui16BitNo, bSet);
        }
    }
}

bool QRelayBase::isBusy()
{
    Q_D(QRelayBase);
    return
        d->logicalEnableMaskNext.count(true) || // not yet started
        d->logicalBusyMask.count(true);         // in progress
}

void QRelayBase::onLowLayerIdle()
{
    Q_D(QRelayBase);
    if(!isBusy())
    {
        // All transactions done -> give notification
        idleCleanup();
        emit idle();
    }
}

// ************************** QRelayUpperBase
QRelayUpperBase::QRelayUpperBase(QObject *parent, QRelayUpperBasePrivate *dp) :
    QRelayBase(parent, dp)

{
    Q_D(QRelayUpperBase);
    // setup out idle timer
    d->m_IdleTimer.setSingleShot(true);
    d->m_IdleTimer.setInterval(0);
    connect(&d->m_IdleTimer, &QTimer::timeout, this, &QRelayUpperBase::onIdleTimer);
}

void QRelayUpperBase::SetLowLayer(QRelayBase *lowRelayLayer)
{
    Q_D(QRelayUpperBase);
    d->lowRelayLayer = lowRelayLayer;
    setupBaseBitmaps(getLogicalRelayCount());
}

void QRelayUpperBase::startSet(quint16 ui16BitNo,
                               bool bSet,
                               bool bForce)
{
    QRelayBase::startSet(ui16BitNo, bSet, bForce);
    Q_D(QRelayUpperBase);
    d->m_IdleTimer.start();
}

const QBitArray &QRelayUpperBase::getLogicalRelayState()
{
    Q_D(QRelayUpperBase);
    // segfault for unset lower layer is ok
    return d->lowRelayLayer->getLogicalRelayState();
}

void QRelayUpperBase::onIdleTimer()
{
    Q_D(QRelayUpperBase);
    // in case low layer is still busy - wait for next idle
    if(d->lowRelayLayer && d->lowRelayLayer->isBusy())
        return;
    // A transaction starts
    process();
}

void QRelayUpperBase::onLowLayerIdle()
{
    Q_D(QRelayBase);
    if(!isBusy())
    {
        // All transactions done -> give notification
        idleCleanup();
        emit idle();
    }
    else
        // Transaction continues
        process();
}
