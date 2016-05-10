#ifndef QRelayMapper_P_H
#define QRelayMapper_P_H

#include <QObject>
#include "relay-mapper.h"

class QRelayMapperRemoteClient;

class QRelayMapperPrivate
{
public:
    QRelayMapperPrivate();
    virtual ~QRelayMapperPrivate();

    // Current state logical (size -> number of logical bits)
    QBitArray logicalSetMask;

    // Next state logical
    QBitArray logicalEnableMaskNext;
    QBitArray logicalSetMaskNext;

    // Setup info
    const TLogicalRelaisEntry *pLogicalInfoArray;

    // Number of physical outputs (extracted from setup info)
    quint16 ui16MaxPhysicalPinHandled;

    // Working vars
    QByteArray arrPinDelayCounter;
    QBitArray logicalBusyMask;

    // Low layer callback
    StartLowLayerSwitchFunction CallbackStartLowLayerSwitch;
};


#endif // QRelayMapper_P_H

