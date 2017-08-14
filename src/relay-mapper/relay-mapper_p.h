#ifndef QRelayMapper_P_H
#define QRelayMapper_P_H

#include "relay-mapper.h"
#include "relay-base_p.h"
#include <QTimer>

class QRelayMapperPrivate : public QRelayBasePrivate
{
public:
    QRelayMapperPrivate();
    virtual ~QRelayMapperPrivate();

    // Setup info
    const TLogicalRelayEntry *pLogicalInfoArray;

    // Low layer callbacks
    RelayQueryLowLayerBusy CallbackQueryLowLayerBusy;
    RelayStartLowLayerSwitchFunction CallbackStartLowLayerSwitch;

    // Current state logical (size -> number of logical bits)
    QBitArray logicalSetMaskCurrent;

    // Number of physical outputs (extracted from setup info)
    quint16 ui16MaxPhysicalPinHandled;

    // Working vars
    QByteArray arrPinDelayCounter;

    // Slice timer
    QTimer m_SliceTimer;
};


#endif // QRelayMapper_P_H

