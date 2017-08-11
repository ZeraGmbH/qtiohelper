#ifndef QRelayBase_P_H
#define QRelayBase_P_H

#include <QObject>
#include "relay-base.h"

class QRelayBasePrivate
{
public:
    QRelayBasePrivate();
    virtual ~QRelayBasePrivate();

    // Next state logical
    QBitArray logicalEnableMaskNext;
    QBitArray logicalSetMaskNext;

    // Current state logical (size -> number of logical bits)
    QBitArray logicalSetMaskCurrent;

    // Low layer callbacks
    RelayQueryLowLayerBusy CallbackQueryLowLayerBusy;
    RelayStartLowLayerSwitchFunction CallbackStartLowLayerSwitch;

    // Working vars
    QBitArray logicalBusyMask;
};


#endif // QRelayBase_P_H

