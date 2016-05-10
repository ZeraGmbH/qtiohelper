#include "relay-mapper.h"
#include "relay-mapper_p.h"

// ************* QRelayMapperPrivate

QRelayMapperPrivate::QRelayMapperPrivate()
{
    pLogicalInfoArray = NULL;
    ui16MaxPhysicalPinHandled = 0;
}

QRelayMapperPrivate::~QRelayMapperPrivate()
{
}

// ************* QRelayMapper

QRelayMapper::QRelayMapper(QObject *parent) :
    QObject(parent),
    d_ptr(new QRelayMapperPrivate())
{
    connect(&m_SliceTimer, &QTimer::timeout, this, &QRelayMapper::onSliceTimer);
}

void QRelayMapper::setup(quint16 ui16LogicalArrayInfoCount,
                         const struct TLogicalRelaisEntry *pLogicalInfoArray,
                         int iMsecSlice,
                         StartLowLayerSwitchFunction CallbackStartLowLayerSwitch)
{
    Q_D(QRelayMapper);

    d->CallbackStartLowLayerSwitch = CallbackStartLowLayerSwitch;

    // Setup logical bitmap sizes / workers
    d->logicalSetMask = QBitArray(ui16LogicalArrayInfoCount);
    d->logicalEnableMaskNext = QBitArray(ui16LogicalArrayInfoCount);
    d->logicalSetMaskNext = QBitArray(ui16LogicalArrayInfoCount);
    d->arrPinDelayCounter.resize(ui16LogicalArrayInfoCount);
    d->logicalBusyMask = QBitArray(ui16LogicalArrayInfoCount);

    // estimate max physical bit number
    for(quint16 ui16Entry=0; ui16Entry<ui16LogicalArrayInfoCount; ui16Entry++)
    {
        d->ui16MaxPhysicalPinHandled = qMax(d->ui16MaxPhysicalPinHandled, pLogicalInfoArray[ui16Entry].ui16OnPosition);
        d->ui16MaxPhysicalPinHandled = qMax(d->ui16MaxPhysicalPinHandled, pLogicalInfoArray[ui16Entry].ui16OffPosition);
    }
    // keep pointer to setup data
    d->pLogicalInfoArray = pLogicalInfoArray;

    // setup out slice timer
    m_SliceTimer.setSingleShot(false);
    m_SliceTimer.setInterval(iMsecSlice);
}

quint16 QRelayMapper::getLogicalRelayCount()
{
    Q_D(QRelayMapper);
    return d->logicalSetMask.count();
}

void QRelayMapper::startSet(const QBitArray& logicalEnableMask,
                            const QBitArray& logicalSetMask,
                            bool bForce)
{
    Q_D(QRelayMapper);
    int iMaxBitEnable = qMin(logicalEnableMask.size(), d->logicalEnableMaskNext.size());
    int iMaxBitSet = qMin(logicalSetMask.size(), d->logicalSetMaskNext.size());
    int iMaxBit = qMin(iMaxBitEnable, iMaxBitSet);

    for(int iBit=0; iBit<iMaxBit; iBit++)
    {
        // set bit data for enabled and (forced) changing bits only
        if( logicalEnableMask.at(iBit) &&
            (bForce || logicalSetMask.at(iBit) != d->logicalSetMask.at(iBit)))
        {
            d->logicalEnableMaskNext.setBit(iBit);
            d->logicalSetMaskNext.setBit(iBit, logicalSetMask.at(iBit));
        }
    }
    if(!m_SliceTimer.isActive())
        m_SliceTimer.start();
}

void QRelayMapper::startSet(quint16 ui16BitNo, bool bSet, bool bForce)
{
    Q_D(QRelayMapper);
    quint16 ui16BitCount = getLogicalRelayCount();
    if(ui16BitNo < ui16BitCount)
    {
        QBitArray logicalEnableMask(ui16BitCount);
        QBitArray logicalSetMask(ui16BitCount);
        logicalEnableMask.setBit(ui16BitNo);
        logicalSetMask.setBit(ui16BitNo, bSet);
        startSet(logicalEnableMask, logicalSetMask, bForce);
    }
}

void QRelayMapper::onSliceTimer()
{
    Q_D(QRelayMapper);
    // prepare output data
    QBitArray physicalEnableMask(d->ui16MaxPhysicalPinHandled+1);
    QBitArray physicalSetMask(d->ui16MaxPhysicalPinHandled+1);
    // loop all logical bits
    bool bLowerIORequested = false;
    for(int iBit=0; iBit<d->logicalSetMaskNext.count(); iBit++)
    {
        const TLogicalRelaisEntry& logicalPinInfoEntry = d->pLogicalInfoArray[iBit];
        // 1. Handle changes since last slice
        if(d->logicalEnableMaskNext.at(iBit))
        {
            // set physical pins
            // by using a default the 'else'-part in the decisions below are not
            // required
            bool bSetOutput = true;
            // Check On/off
            quint16 ui16OutBitPosition;
            if(d->logicalSetMaskNext.at(iBit))
            {
                ui16OutBitPosition = logicalPinInfoEntry.ui16OnPosition;
                // ON state same for bistable/monostable
                if(logicalPinInfoEntry.ui8Flags & (1<<RELAIS_PHYS_NEG_ON))
                    bSetOutput = false;
            }
            else
            {
                // OFF state Different behaviour for uni-/bistable
                if(logicalPinInfoEntry.ui8Flags & (1<<RELAIS_BISTABLE))
                {
                    ui16OutBitPosition = logicalPinInfoEntry.ui16OffPosition;
                    if(logicalPinInfoEntry.ui8Flags & (1<<RELAIS_PHYS_NEG_OFF))
                        bSetOutput = false;
                }
                else
                {
                    // uinstable sets only ON pin
                    ui16OutBitPosition = logicalPinInfoEntry.ui16OnPosition;
                    if((logicalPinInfoEntry.ui8Flags & (1<<RELAIS_PHYS_NEG_ON)) == 0)
                        bSetOutput = false;
                }
            }
            // Do set output
            physicalEnableMask.setBit(ui16OutBitPosition, true);
            physicalSetMask.setBit(ui16OutBitPosition, bSetOutput);

            // Setup timer (+1: timer is deleted below - so zero values finish here)
            d->arrPinDelayCounter[iBit] = logicalPinInfoEntry.ui8OnTime+1;
            // mark busy
            d->logicalBusyMask.setBit(iBit);
            // keep state
            d->logicalSetMask.setBit(iBit, d->logicalSetMaskNext.at(iBit));
            // set handled
            d->logicalEnableMaskNext.clearBit(iBit);
            // we need a low layer transaction
            bLowerIORequested = true;
        }
        // 2. Handle busy logical bits
        if(d->logicalBusyMask.at(iBit))
        {
            // pin delay finished
            quint8 ui8Counter = d->arrPinDelayCounter[iBit];
            ui8Counter--;
            d->arrPinDelayCounter[iBit] = ui8Counter;
            if(ui8Counter == 0)
            {
                // Only bistable relais have to be touched
                if(logicalPinInfoEntry.ui8Flags & (1<<RELAIS_BISTABLE))
                {
                    uint16_t ui16OutBitPosition;
                    bool bSetOutput = true;
                    if(d->logicalSetMask.at(iBit))
                    {
                        // It is ON
                        ui16OutBitPosition = logicalPinInfoEntry.ui16OnPosition;
                        if((logicalPinInfoEntry.ui8Flags & (1<<RELAIS_PHYS_NEG_ON)) == 0)
                            bSetOutput = false;
                    }
                    else
                    {
                        // It is OFF
                        ui16OutBitPosition = logicalPinInfoEntry.ui16OffPosition;
                        if((logicalPinInfoEntry.ui8Flags & (1<<RELAIS_PHYS_NEG_OFF)) == 0)
                            bSetOutput = false;
                    }

                    // Do set output
                    physicalEnableMask.setBit(ui16OutBitPosition, true);
                    physicalSetMask.setBit(ui16OutBitPosition, bSetOutput);
                    // we need a low layer transaction
                    bLowerIORequested = true;
                }
                // Mark this bit as finished
                d->logicalBusyMask.clearBit(iBit);
            }
        }
    }
    bool bLowerLayerBusy = false;
    if(bLowerIORequested)
    {
        if(d->CallbackStartLowLayerSwitch)
            bLowerLayerBusy = d->CallbackStartLowLayerSwitch(physicalEnableMask, physicalSetMask, this);
    }
    if(!bLowerLayerBusy)
        onLowLayerFinish(this);
}

void QRelayMapper::onLowLayerFinish(const QObject *pSignalHandler)
{
    if(pSignalHandler == this)
    {
        Q_D(QRelayMapper);
        // check if all pins are idle
        bool bOneOrMoreBusy = false;
        for(int iBit=0; iBit<d->logicalSetMask.count(); iBit++)
        {
            if(d->logicalBusyMask.at(iBit))
            {
                bOneOrMoreBusy = true;
                break;
            }
        }
        if(!bOneOrMoreBusy)
        {
            m_SliceTimer.stop();
            emit idle();
        }
    }
}
