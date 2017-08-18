#include <QDebug>
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
    for(int ui16BitNo=0;
        ui16BitNo<logicalEnableMask.size() && ui16BitNo<getLogicalRelayCount();
        ui16BitNo++)
    {
        // set bit data for enabled and (forced) changing bits only
        if( logicalEnableMask.at(ui16BitNo) &&
            (bForce || logicalSetMask.at(ui16BitNo) != getLogicalRelayState().at(ui16BitNo)))
        {
            d->logicalEnableMaskNext.setBit(ui16BitNo);
            d->logicalSetMaskNext.setBit(ui16BitNo, logicalSetMask.at(ui16BitNo));
        }
    }
}

void QRelayBase::startSet(quint16 ui16BitNo,
                          bool bSet,
                          bool bForce)
{
    Q_D(QRelayBase);
    quint16 ui16LogicalArrayCount = getLogicalRelayCount();
    QBitArray logicalEnableMask = QBitArray(ui16LogicalArrayCount);
    QBitArray logicalSetMask = QBitArray(ui16LogicalArrayCount);
    if(ui16BitNo < ui16LogicalArrayCount)
    {
        logicalEnableMask.setBit(ui16BitNo);
        logicalSetMask.setBit(ui16BitNo, bSet);
    }
    else
        qWarning() << "QRelayBase::startSet: ui16BitNo out of range:" << ui16BitNo << "max:" << getLogicalRelayCount();
    // even if no valid bit was set: start transaction so we get an idle signal
    startSetMulti(logicalEnableMask, logicalSetMask, bForce);
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
        // All transactions done -> give notification
        emit idle();
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
    // reconnect?
    if(d->lowRelayLayer)
        // avoid multiple slot calls - we allow only one lower layer anyway
        QObject::disconnect(d->lowRelayLayer, &QRelayBase::idle, this, &QRelayUpperBase::onLowLayerIdle);

    d->lowRelayLayer = lowRelayLayer;
    QObject::connect(lowRelayLayer, &QRelayBase::idle, this, &QRelayUpperBase::onLowLayerIdle);
    setupBaseBitmaps(getLogicalRelayCount());
}

void QRelayUpperBase::startSetMulti(const QBitArray& logicalEnableMask,
                                    const QBitArray& logicalSetMask,
                                    bool bForce)
{
    QRelayBase::startSetMulti(logicalEnableMask, logicalSetMask, bForce);
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
    // Does transaction start?
    if(!process())
        // No -> give notification
        emit idle();
}

void QRelayUpperBase::onLowLayerIdle()
{
    Q_D(QRelayBase);
    // Transaction continues?
    if(isBusy() && !process())
        // No -> give notification
        emit idle();
}
