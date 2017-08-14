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

    // Working vars
    QBitArray logicalBusyMask;
};

class QRelayUpperBasePrivate : public QRelayBasePrivate
{
public:
    QRelayUpperBasePrivate();

    // collect all startSet and start on idle
    QTimer m_IdleTimer;
    // Low layer
    QRelayBase *lowRelayLayer;

};

#endif // QRelayBase_P_H

