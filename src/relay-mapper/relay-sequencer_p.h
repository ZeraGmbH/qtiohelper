#ifndef QRelaySequencer_P_H
#define QRelaySequencer_P_H

#include "relay-base_p.h"
#include "relay-sequencer.h"
#include <QList>

enum enRelaySequencerSwitchState
{
    SEQUENCER_STATE_IDLE = 0,
    SEQUENCER_STATE_2ND,
    SEQUENCER_STATE_END

};

class QRelaySequencerPrivate : public QRelayUpperBasePrivate
{
public:
    QRelaySequencerPrivate();
    virtual ~QRelaySequencerPrivate();

    // Sequence groups
    QList<TRelaySequencerGroup> listGroups;

    // current state
    enum enRelaySequencerSwitchState relaySequencerSwitchState;
    // bit set during 2nd state (values those fond at input)
    QBitArray enableMask2;
    QBitArray setMask2;
};


#endif // QRelaySequencer_P_H

