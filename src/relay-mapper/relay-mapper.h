#ifndef QRelayMapper_H
#define QRelayMapper_H

#include <QObject>
#include <QBitArray>
#include <functional>
#include "relay_global.h"
#include "relay-base.h"

struct TLogicalRelaisEntry
{
    quint16 ui16OnPosition; // Bit position to switch on
    quint16 ui16OffPosition;// Bit position to switch off
    quint8 ui8Flags;        // binary flags (see enLogicalRelaisFlags)
    quint8 ui8OnTime;       // bistable on time in ticks
};

enum enLogicalRelaisFlags
{
    RELAIS_LOG_VALID,       // Only relays set by RELAIS_PHYS_FLAGS are
                            // marked as valid. RelaisMapper keeps validity
                            // mask in ui8RelaisMapperValidLogicalMask for other
                            // layers to ignore not defined relays.
    RELAIS_BISTABLE,        // If set: bistable
    RELAIS_PHYS_NEG_ON,     // If set a PHYSICAL output is active at low level
                            // For bistable relays: a switching transaction is
                            // initiated by setting the output to LOW level
    RELAIS_PHYS_NEG_OFF,	// ignored for monostable relays / for bistable
                            // off-pin is negative active
};

// Helper macro for physical flags defintion
#define RELAIS_PHYS_FLAGS(bBistable, bOnNeg, bOffNeg)   \
    (1<<RELAIS_LOG_VALID)           |                   \
    (bBistable<<RELAIS_BISTABLE)    |                   \
    (bOnNeg<<RELAIS_PHYS_NEG_ON)    |                   \
    (bOffNeg<<RELAIS_PHYS_NEG_OFF)


class QRelayMapperPrivate;

class QTRELAYSSHARED_EXPORT QRelayMapper : public QRelayBase
{
    Q_OBJECT
public:
    QRelayMapper(QObject *parent = Q_NULLPTR);
    void setup(quint16 ui16LogicalArrayInfoCount,
               const struct TLogicalRelaisEntry *pLogicalInfoArray,
               int iMsecSlice,
               RelayStartLowLayerSwitchFunction CallbackStartLowLayerSwitch);

    virtual void startSet(quint16 ui16BitNo,
                          bool bSet,
                          bool bForce = false);

protected:
    virtual void idleCleanup();

private slots:
    void onSliceTimer();

private:
    Q_DECLARE_PRIVATE(QRelayMapper)
};

#endif // QRelayMapper_H
