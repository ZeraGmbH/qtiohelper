#ifndef QRelayMapper_H
#define QRelayMapper_H

#include <QObject>
#include <QBitArray>
#include <functional>
#include "relay_global.h"
#include "relay-base.h"

struct TLogicalRelayEntry
{
    quint16 ui16OnPosition; // Bit position to switch on
    quint16 ui16OffPosition;// Bit position to switch off
    quint8 ui8Flags;        // binary flags (see enLogicalRelayFlags)
    quint8 ui8OnTime;       // bistable on time in ticks
};

enum enLogicalRelayFlags
{
    RELAY_LOG_VALID,       // Only relays set by RELAY_PHYS_FLAGS are
                            // marked as valid. RelayMapper keeps validity
                            // mask in ui8RelayMapperValidLogicalMask for other
                            // layers to ignore not defined relays.
    RELAY_BISTABLE,        // If set: bistable
    RELAY_PHYS_NEG_ON,     // If set a PHYSICAL output is active at low level
                            // For bistable relays: a switching transaction is
                            // initiated by setting the output to LOW level
    RELAY_PHYS_NEG_OFF,	// ignored for monostable relays / for bistable
                            // off-pin is negative active
};

// Helper macro for physical flags defintion
#define RELAY_PHYS_FLAGS(bBistable, bOnNeg, bOffNeg)   \
    (1<<RELAY_LOG_VALID)           |                   \
    (bBistable<<RELAY_BISTABLE)    |                   \
    (bOnNeg<<RELAY_PHYS_NEG_ON)    |                   \
    (bOffNeg<<RELAY_PHYS_NEG_OFF)

// Callback function for asking if lower layer is busy
typedef std::function<bool()> RelayQueryLowLayerBusy;

// Callback function for switching lower layer
//  * Only bits which are set in EnableMask are handled - other bits in SetMask are ignored
//  * NON-BLOCKING:
//      -return value true (=busy)
//      -low layer signal idle should be connected to onLowLayerIdle
//  * BLOCKING:
//      -return value false - onLowLayerIdle within current layer itself
typedef std::function<bool(const QBitArray& EnableMask, const QBitArray& SetMask, const QObject *pSignalHandler)> RelayStartLowLayerSwitchFunction;

class QRelayMapperPrivate;


// Within the relay framework QRelayMapper is somewhat special: It is always
// the lowest layer in the relay layer stack
class QTRELAYSSHARED_EXPORT QRelayMapper : public QRelayBase
{
    Q_OBJECT
public:
    QRelayMapper(QObject *parent = Q_NULLPTR);
    void setup(quint16 ui16LogicalArrayInfoCount,
               const struct TLogicalRelayEntry *pLogicalInfoArray,
               int iMsecSlice,
               RelayStartLowLayerSwitchFunction CallbackStartLowLayerSwitch);
    // setup busy callback - our lower layer is usually not derived from QRelayBase so connect
    // by callback
    void setupCallbackLowLayerBusy(RelayQueryLowLayerBusy callback);

    virtual void startSetMulti(const QBitArray& logicalEnableMask,
                               const QBitArray& logicalSetMask,
                               bool bForce = false);
    virtual const QBitArray& getLogicalRelayState();

private slots:
    void onSliceTimer();

private:
    Q_DECLARE_PRIVATE(QRelayMapper)
};

#endif // QRelayMapper_H
