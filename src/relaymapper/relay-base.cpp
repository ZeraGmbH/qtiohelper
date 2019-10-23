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
    bypassToLowerLayerActive = false;
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
    return static_cast<quint16>(getLogicalRelayState().size());
}

void QRelayBase::setupBaseBitmaps(quint16 ui16LogicalArrayInfoCount)
{
    Q_D(QRelayBase);
    d->logicalEnableMaskNext.fill(false, ui16LogicalArrayInfoCount);
    d->logicalSetMaskNext.fill(false, ui16LogicalArrayInfoCount);
    d->logicalDirtyMask.fill(false, ui16LogicalArrayInfoCount);
    d->logicalTargetMask.fill(false, ui16LogicalArrayInfoCount);
}

void QRelayBase::startSetMulti(const QBitArray& logicalEnableMask,
                               const QBitArray& logicalSetMask,
                               bool bForce)
{
    Q_D(QRelayBase);
    for(int ui16BitNo=0;
        ui16BitNo<logicalEnableMask.size() && ui16BitNo<getLogicalRelayCount();
        ui16BitNo++) {
        // set bit data for enabled and (forced) changing bits only
        if( logicalEnableMask.testBit(ui16BitNo) &&
            (bForce || logicalSetMask.testBit(ui16BitNo) != getLogicalRelayState().testBit(ui16BitNo))) {
            d->logicalEnableMaskNext.setBit(ui16BitNo);
            d->logicalSetMaskNext.setBit(ui16BitNo, logicalSetMask.testBit(ui16BitNo));
        }
    }
}

void QRelayBase::startSet(quint16 ui16BitNo,
                          bool bSet,
                          bool bForce)
{
    quint16 ui16LogicalArrayCount = getLogicalRelayCount();
    QBitArray logicalEnableMask(ui16LogicalArrayCount);
    QBitArray logicalSetMask(ui16LogicalArrayCount);
    if(ui16BitNo < ui16LogicalArrayCount) {
        logicalEnableMask.setBit(ui16BitNo);
        logicalSetMask.setBit(ui16BitNo, bSet);
    }
    else {
        qWarning("QRelayBase::startSet: ui16BitNo out of range: %u / max: %u", ui16BitNo,getLogicalRelayCount() );
    }
    // even if no valid bit was set: start transaction so we get an idle signal
    startSetMulti(logicalEnableMask, logicalSetMask, bForce);
}

bool QRelayBase::startNextTransaction()
{
    Q_D(QRelayBase);
    // we are not busy check for next transaction
    if(d->logicalDirtyMask.count(true) == 0 && d->logicalEnableMaskNext.count(true)) {
        // calculate target state
        d->logicalTargetMask = getLogicalRelayState();
        for(quint16 ui16Bit=0; ui16Bit<getLogicalRelayCount(); ui16Bit++) {
            if(d->logicalEnableMaskNext.testBit(ui16Bit)) {
                d->logicalTargetMask.setBit(ui16Bit, d->logicalSetMaskNext.testBit(ui16Bit));
            }
        }
        // calculate bit difference mask
        d->logicalDirtyMask = getLogicalRelayState() ^ d->logicalSetMaskNext;
        // filter enabled
        d->logicalDirtyMask &= d->logicalEnableMaskNext;
        // everything else goes in next transaction
        d->logicalEnableMaskNext.fill(false);
    }
    return d->logicalDirtyMask.count(true);
}

bool QRelayBase::isBusy()
{
    Q_D(QRelayBase);
    return
        d->logicalEnableMaskNext.count(true) || // not yet started
        d->logicalDirtyMask.count(true);        // in progress
}

void QRelayBase::onLowLayerIdle()
{
    if(!isBusy()) {
        // All transactions done -> give notification
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

void QRelayUpperBase::setLowLayer(QRelayBase *lowRelayLayer)
{
    Q_D(QRelayUpperBase);
    // reconnect?
    if(d->lowRelayLayer) {
        // avoid multiple slot calls - we allow only one lower layer anyway
        QObject::disconnect(d->lowRelayLayer, &QRelayBase::idle, this, &QRelayUpperBase::onLowLayerIdle);
    }

    d->lowRelayLayer = lowRelayLayer;
    if(lowRelayLayer) {
        QObject::connect(lowRelayLayer, &QRelayBase::idle, this, &QRelayUpperBase::onLowLayerIdle, Qt::DirectConnection);
        setupBaseBitmaps(getLogicalRelayCount());
    }
    else {
        setupBaseBitmaps(0);
    }
}

void QRelayUpperBase::startSetMulti(const QBitArray& logicalEnableMask,
                                    const QBitArray& logicalSetMask,
                                    bool bForce)
{
    Q_D(QRelayUpperBase);
    // setting bForce is intended to set initial relay state -> bypass to lowest layer (mapper)
    if(bForce && d->lowRelayLayer) {
        d->lowRelayLayer->startSetMulti(logicalEnableMask, logicalSetMask, bForce);
        d->bypassToLowerLayerActive = true;
    }
    else {
        QRelayBase::startSetMulti(logicalEnableMask, logicalSetMask, bForce);
        d->m_IdleTimer.start();
    }
}


const QBitArray &QRelayUpperBase::getLogicalRelayState()
{
    Q_D(QRelayUpperBase);
    // segfault for unset lower layer is ok
    return d->lowRelayLayer->getLogicalRelayState();
}

bool QRelayUpperBase::isBusy()
{
    Q_D(QRelayUpperBase);
    return d->lowLayerTransactionStarted || QRelayBase::isBusy();
}

void QRelayUpperBase::onIdleTimer()
{
    Q_D(QRelayUpperBase);
    // in case low layer is still busy - wait for next idle
    if(d->lowRelayLayer && d->lowRelayLayer->isBusy()) {
        return;
    }
    // does this layer start a low layer transaction?
    d->lowLayerTransactionStarted = process();
    if(!d->lowLayerTransactionStarted) {
        // no -> finish -> give notification
        emit idle();
    }
}

void QRelayUpperBase::onLowLayerIdle()
{
    Q_D(QRelayUpperBase);
    // special bypass case
    if(d->bypassToLowerLayerActive) {
        d->bypassToLowerLayerActive = false;
        emit idle();
    }
    // transaction pending?
    if(isBusy()) {
        // does this layer start a low layer transaction?
        d->lowLayerTransactionStarted = process();
        if(!d->lowLayerTransactionStarted) {
            // no -> finish -> give notification
            emit idle();
        }
    }
}
