#ifndef QRelaySequencer_P_H
#define QRelaySequencer_P_H

#include "relay-base_p.h"
#include "relay-sequencer.h"
#include <QTimer>

class QRelaySequencerPrivate : public QRelayBasePrivate
{
public:
    QRelaySequencerPrivate();
    virtual ~QRelaySequencerPrivate();

    // sequence (overlapped) type
    enum enRelaisSequencerSwitchTypes relaisSequencerSwitchType;
    // logical relays in this group
    QVector<quint16> arrui16MemberLogRelayNums;

    // collect all startSet and start on idle
    QTimer m_IdleTimer;
};


#endif // QRelaySequencer_P_H

