#ifndef QRelayMapper_H
#define QRelayMapper_H

#include <QObject>
#include <QTimer>
#include <QBitArray>
#include <functional>
#include "relay-mapper_global.h"

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

// Callback function for lower layer
//   * return value signals non-blocking or we'll wait for onLowLayerFinish
//   * Only bits which are set in EnableMask are handled - other bits in SetMask are ignored
typedef std::function<bool(const QBitArray& EnableMask, const QBitArray& SetMask, const QObject *pSignalHandler)> StartLowLayerSwitchFunction;

class QTRELAYSSHARED_EXPORT QRelayMapper : public QObject
{
    Q_OBJECT
public:
    QRelayMapper(QObject *parent = NULL);
    void setup(quint16 ui16LogicalArrayInfoCount,
               const struct TLogicalRelaisEntry *pLogicalInfoArray,
               int iMsecSlice,
               StartLowLayerSwitchFunction CallbackStartLowLayerSwitch);
    quint16 getLogicalRelayCount();

    void startSet(const QBitArray& logicalEnableMask,
                  const QBitArray& logicalSetMask,
                  bool bForce = false);
    void startSet(quint16 ui16BitNo,
                  bool bSet,
                  bool bForce = false);

signals:
    void idle();

public slots:
    void onLowLayerFinish(const QObject *pSignalHandler);

private slots:
    void onSliceTimer();

private:
    QTimer m_SliceTimer;

    QRelayMapperPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QRelayMapper)
};

#endif // QRelayMapper_H
