#include "relay-base.h"
#include "relay-base_p.h"

// ************************** QRelayBasePrivate

QRelayBasePrivate::QRelayBasePrivate()
{
    CallbackQueryLowLayerBusy = nullptr;
    CallbackStartLowLayerSwitch = nullptr;
}

QRelayBasePrivate::~QRelayBasePrivate()
{
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

void QRelayBase::setupCallbackLowLayerBusy(RelayQueryLowLayerBusy callback)
{
    Q_D(QRelayBase);

    d->CallbackQueryLowLayerBusy = callback;
}

void QRelayBase::setup(quint16 ui16LogicalArrayInfoCount, RelayStartLowLayerSwitchFunction CallbackStartLowLayerSwitch)
{
    Q_D(QRelayBase);

    d->CallbackStartLowLayerSwitch = CallbackStartLowLayerSwitch;
    d->logicalEnableMaskNext = QBitArray(ui16LogicalArrayInfoCount);
    d->logicalSetMaskNext = QBitArray(ui16LogicalArrayInfoCount);
    d->logicalBusyMask = QBitArray(ui16LogicalArrayInfoCount);
    d->logicalSetMaskCurrent = QBitArray(ui16LogicalArrayInfoCount);
}

quint16 QRelayBase::getLogicalRelayCount()
{
    Q_D(QRelayBase);

    return d->logicalSetMaskCurrent.count();
}

const QBitArray &QRelayBase::getLogicalRelayState()
{
    Q_D(QRelayBase);
    return d->logicalSetMaskCurrent;
}

void QRelayBase::startSetMulti(const QBitArray& logicalEnableMask,
                               const QBitArray& logicalSetMask,
                               bool bForce)
{
    Q_D(QRelayBase);

    for(int iBit=0;
        iBit<logicalEnableMask.size() && iBit<getLogicalRelayCount();
        iBit++)
        startSet(iBit, iBit<logicalSetMask.size() ? logicalSetMask.at(iBit) : false, bForce);
}

void QRelayBase::startSet(quint16 ui16BitNo,
                          bool bSet,
                          bool bForce)
{
    Q_D(QRelayBase);

    if(ui16BitNo < getLogicalRelayCount())
    {
        if(bForce || bSet != d->logicalSetMaskCurrent.at(ui16BitNo))
        {
            d->logicalEnableMaskNext.setBit(ui16BitNo);
            d->logicalSetMaskNext.setBit(ui16BitNo, bSet);
        }
    }
}

bool QRelayBase::isBusy()
{
    Q_D(QRelayBase);

    QBitArray allZeros = QBitArray(getLogicalRelayCount());
    return
        d->logicalEnableMaskNext != allZeros && // not yet started
        d->logicalBusyMask != allZeros;         // in progress
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
