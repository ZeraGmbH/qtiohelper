#include "relay-sequencer.h"
#include "relay-sequencer_p.h"

// ************************** QRelaySequencerPrivate

QRelaySequencerPrivate::QRelaySequencerPrivate()
{
    relaySequencerSwitchState = SEQUENCER_STATE_IDLE;
}

QRelaySequencerPrivate::~QRelaySequencerPrivate()
{
}

// ************************** QRelaySequencer

QRelaySequencer::QRelaySequencer(QObject *parent) :
    QRelayUpperBase(parent, new QRelaySequencerPrivate())
{
    Q_D(QRelaySequencer);
}

void QRelaySequencer::AddGroup(TRelaySequencerGroup &group)
{
    Q_D(QRelaySequencer);
    d->listGroups.append(group);
}

void QRelaySequencer::setupBaseBitmaps(quint16 ui16LogicalArrayInfoCount)
{
    QRelayUpperBase::setupBaseBitmaps(ui16LogicalArrayInfoCount);
    Q_D(QRelaySequencer);
    d->enableMask2 = QBitArray(ui16LogicalArrayInfoCount);
    d->setMask2 = QBitArray(ui16LogicalArrayInfoCount);
}

bool QRelaySequencer::process()
{
    Q_D(QRelaySequencer);
    bool idleOut = false;
    switch(d->relaySequencerSwitchState)
    {
    case SEQUENCER_STATE_IDLE:
        // Start a next transaction?
        if(d->logicalEnableMaskNext.count(true))
        {
            // Buffer enable state -> busy to allow next transaction drop ins
            d->logicalBusyMask = d->logicalEnableMaskNext;
            d->logicalEnableMaskNext.fill(false);
            d->setMask2 = d->logicalSetMaskNext;
            // calculate bit difference mask
            QBitArray dirtyMask = getLogicalRelayState() ^ d->logicalSetMaskNext;
            // filter enabled
            dirtyMask &= d->logicalBusyMask;
            quint16 ui16Bit;
            // Nothing to be done?
            if(dirtyMask.count(true) == 0)
                idleOut = true;
            else
            {
                QBitArray enableMask1 = QBitArray(getLogicalRelayCount());
                QBitArray setMask1 = d->logicalSetMaskNext;
                // loop all bits
                for(ui16Bit=0; ui16Bit<getLogicalRelayCount(); ui16Bit++)
                {
                    if(dirtyMask.at(ui16Bit))
                    {
                        enum enRelaySequencerSwitchTypes switchType = SWITCH_TRANSPARENT;
                        // Find group and handle full group cases
                        for(int group=0; group<d->listGroups.size(); group++)
                        {
                            if(d->listGroups[group].arrui16MemberLogRelayNums.contains(ui16Bit))
                            {
                                switchType = d->listGroups[group].relaySequencerSwitchType;
                                // Full on/off groups handled here
                                if(switchType == SWITCH_PASS_OFF || switchType == SWITCH_PASS_ON)
                                {
                                    for(int groupMember=0; groupMember<d->listGroups[group].arrui16MemberLogRelayNums.size(); groupMember++)
                                    {
                                        quint16 ui16GroupBit = d->listGroups[group].arrui16MemberLogRelayNums[groupMember];
                                        setMask1.setBit(ui16GroupBit, switchType == SWITCH_PASS_ON);
                                        enableMask1.setBit(ui16GroupBit);
                                        d->enableMask2.setBit(ui16GroupBit);
                                    }
                                }
                                break;
                            }
                        }
                        // set next bit info
                        switch(switchType)
                        {
                        case SWITCH_TRANSPARENT:
                        default:
                            // set at 2nd transaction
                            d->enableMask2.setBit(ui16Bit);
                            break;
                        case SWITCH_OVERLAPPED_ON:
                            if(d->logicalSetMaskNext.at(ui16Bit))
                                // switch on during first transation
                                enableMask1.setBit(ui16Bit);
                            else
                                // switch off at 2nd transaction
                                d->enableMask2.setBit(ui16Bit);
                            break;
                        case SWITCH_OVERLAPPED_OFF:
                            if(!d->logicalSetMaskNext.at(ui16Bit))
                                // switch off during first transation
                                enableMask1.setBit(ui16Bit);
                            else
                                // switch on at 2nd transaction
                                d->enableMask2.setBit(ui16Bit);
                            break;
                        case SWITCH_PASS_ON:
                        case SWITCH_PASS_OFF:
                            // andled above
                            break;
                        }
                    }
                }
                // prepare next state
                d->relaySequencerSwitchState = SEQUENCER_STATE_2ND;
                // start low layer action
                d->lowRelayLayer->startSetMulti(enableMask1, setMask1);
            }

        }
        break;
    case SEQUENCER_STATE_2ND:
        // prepare next state
        d->relaySequencerSwitchState = SEQUENCER_STATE_END;
        // start low layer action
        d->lowRelayLayer->startSetMulti(d->enableMask2, d->setMask2);
        break;
    case SEQUENCER_STATE_END:
        // all work is done
        idleOut = true;
        break;
    }
    return !idleOut;
}

void QRelaySequencer::idleCleanup()
{
    Q_D(QRelaySequencer);
    d->relaySequencerSwitchState = SEQUENCER_STATE_IDLE;
    d->logicalBusyMask.fill(false);
}
