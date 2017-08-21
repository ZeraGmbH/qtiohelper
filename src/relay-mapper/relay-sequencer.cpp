#include "relay-sequencer.h"
#include "relay-sequencer_p.h"
#include "QDebug"

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

bool QRelaySequencer::AddGroup(const TRelaySequencerGroup &group)
{
    Q_D(QRelaySequencer);
    int relay;
    // relays shall not be members of more than one group - check that
    bool checkOK = true;
    for(relay=0; relay<group.arrui16MemberLogRelayNums.size(); relay++)
    {
        quint16 relayNum = group.arrui16MemberLogRelayNums[relay];
        if(d->insertedRelays.contains(relayNum))
        {
            checkOK = false;
            qCritical() << "Relay" << relayNum << "is already member of a relay-sequencer group. Group will be ignored!!";
        }
    }
    if(checkOK)
    {
        d->vecGroups.append(group);
        for(relay=0; relay<group.arrui16MemberLogRelayNums.size(); relay++)
            d->insertedRelays.insert(group.arrui16MemberLogRelayNums[relay]);
    }
    return checkOK;
}

void QRelaySequencer::setupBaseBitmaps(quint16 ui16LogicalArrayInfoCount)
{
    Q_D(QRelaySequencer);
    QRelayUpperBase::setupBaseBitmaps(ui16LogicalArrayInfoCount);
    d->enableMask2.fill(false, ui16LogicalArrayInfoCount);
}

bool QRelaySequencer::process()
{
    Q_D(QRelaySequencer);
    bool startedLowLayerTransaction = false;
    switch(d->relaySequencerSwitchState)
    {
    case SEQUENCER_STATE_IDLE:
        // Start a next transaction?
        if(startNextTransaction())
        {
            QBitArray enableMask1(getLogicalRelayCount());
            QBitArray setMask1 = d->logicalSetMaskNext;
            // We need a local copy - bits are deleted below
            QBitArray dirtyMask = d->logicalDirtyMask;
            // loop all bits
            for(quint16 ui16Bit=0; ui16Bit<getLogicalRelayCount(); ui16Bit++)
            {
                if(dirtyMask.testBit(ui16Bit))
                {
                    enum enRelaySequencerSwitchTypes switchType = SWITCH_TRANSPARENT;
                    // Find group and handle full group cases
                    for(int group=0; group<d->vecGroups.size(); group++)
                    {
                        if(d->vecGroups[group].arrui16MemberLogRelayNums.contains(ui16Bit))
                        {
                            switchType = d->vecGroups[group].relaySequencerSwitchType;
                            // Full on/off groups handled here
                            if(switchType == SWITCH_PASS_OFF || switchType == SWITCH_PASS_ON)
                            {
                                for(int groupMember=0;
                                    groupMember<d->vecGroups[group].arrui16MemberLogRelayNums.size();
                                    groupMember++)
                                {
                                    quint16 ui16GroupBit = d->vecGroups[group].arrui16MemberLogRelayNums[groupMember];
                                    setMask1.setBit(ui16GroupBit, switchType == SWITCH_PASS_ON);
                                    enableMask1.setBit(ui16GroupBit);
                                    bool setBit =
                                            (switchType == SWITCH_PASS_ON && d->logicalTargetMask.testBit(ui16GroupBit) == false) ||
                                            (switchType == SWITCH_PASS_OFF && d->logicalTargetMask.testBit(ui16GroupBit) == true);
                                    d->enableMask2.setBit(ui16GroupBit, setBit);
                                    // The bits handled here can be considered as done
                                    // (a relay can be member only in one group). So avoid
                                    // rechecking same bits over and over in main loop
                                    dirtyMask.clearBit(ui16GroupBit);
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
                        if(d->logicalSetMaskNext.testBit(ui16Bit))
                            // switch on during first transation
                            enableMask1.setBit(ui16Bit);
                        else
                            // switch off at 2nd transaction
                            d->enableMask2.setBit(ui16Bit);
                        break;
                    case SWITCH_OVERLAPPED_OFF:
                        if(!d->logicalSetMaskNext.testBit(ui16Bit))
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
            startedLowLayerTransaction = true;
        }
        break;
    case SEQUENCER_STATE_2ND:
        // prepare next state
        d->relaySequencerSwitchState = SEQUENCER_STATE_END;
        // start low layer action
        d->lowRelayLayer->startSetMulti(d->enableMask2, d->logicalTargetMask);
        startedLowLayerTransaction = true;
        break;
    case SEQUENCER_STATE_END:
        // all work is done
        d->relaySequencerSwitchState = SEQUENCER_STATE_IDLE;
        d->logicalDirtyMask.fill(false);
        break;
    }
    return startedLowLayerTransaction;
}
