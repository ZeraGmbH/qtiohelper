#ifndef QRelayBase_P_H
#define QRelayBase_P_H

#include <QTimer>
#include "relay-base.h"

class QRelayBasePrivate
{
public:
    QRelayBasePrivate();
    virtual ~QRelayBasePrivate();

    // Next state logical
    QBitArray logicalEnableMaskNext;
    QBitArray logicalSetMaskNext;

    // Bits to change in this transaction
    QBitArray logicalDirtyMask;
    // The after transaction target bitmask
    QBitArray logicalTargetMask;
};

class QRelayUpperBasePrivate : public QRelayBasePrivate
{
public:
    QRelayUpperBasePrivate();

    // collect all startSet and start on idle
    QTimer m_IdleTimer;
    // Low layer
    QRelayBase *lowRelayLayer;
    // Set when startSet is called with bForce=true and signals transparent
    // bypassing
    bool bypassToLowerLayerActive;
    // we started a low layer transaction
    bool lowLayerTransactionStarted;
};

#endif // QRelayBase_P_H

