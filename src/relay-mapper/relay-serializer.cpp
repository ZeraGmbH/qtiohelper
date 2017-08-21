#include "relay-serializer.h"
#include "relay-serializer_p.h"
#include "QDebug"

// ************************** QRelaySerializerPrivate

QRelaySerializerPrivate::QRelaySerializerPrivate()
{
}

QRelaySerializerPrivate::~QRelaySerializerPrivate()
{
}

// ************************** QRelaySerializer

QRelaySerializer::QRelaySerializer(QObject *parent) :
    QRelayUpperBase(parent, new QRelaySerializerPrivate())
{
    Q_D(QRelaySerializer);
}

bool QRelaySerializer::AddGroup(const TRelaySerializerGroup &group)
{
    Q_D(QRelaySerializer);
    int relay;
    bool checkOK = true;
    for(relay=0; relay<group.arrSerializerRelayData.size(); relay++)
    {
        // relays shall not be members of more than one group - check that
        quint16 relayNum = group.arrSerializerRelayData[relay].relayNum;
        if(d->insertedRelays.contains(relayNum))
        {
            checkOK = false;
            qCritical() << "Relay" << relayNum << "is already member of a relay-serializer group. Group will be ignored!!";
        }
        // relay's current must be less or equal than power supply's current
        if(group.arrSerializerRelayData[relay].supplyCurrentOff > group.powerSupplyMaxCurrent ||
           group.arrSerializerRelayData[relay].supplyCurrentOn > group.powerSupplyMaxCurrent)
        {
            checkOK = false;
            qCritical() << "Relay" << relayNum << ": current is larger than supply" << "Group will be ignored!!";
        }
    }
    if(checkOK)
    {
        d->vecGroups.append(group);
        for(relay=0; relay<group.arrSerializerRelayData.size(); relay++)
            d->insertedRelays.insert(group.arrSerializerRelayData[relay].relayNum);
    }
    return checkOK;
}

void QRelaySerializer::setupBaseBitmaps(quint16 ui16LogicalArrayInfoCount)
{
    Q_D(QRelaySerializer);
    QRelayUpperBase::setupBaseBitmaps(ui16LogicalArrayInfoCount);
    // Revisit remove??
}

bool QRelaySerializer::process()
{
    Q_D(QRelaySerializer);
    bool startedLowLayerTransaction = false;
    // start new transaction
    startNextTransaction();
    // we have bits to switch left over?
    if(d->logicalDirtyMask.count(true))
    {
        QVector<float> groupLoadCurrent(d->vecGroups.size());
        QBitArray nextEnableMask(getLogicalRelayCount());
        for(quint16 ui16Bit=0; ui16Bit<getLogicalRelayCount(); ui16Bit++)
        {
            if(d->logicalDirtyMask.testBit(ui16Bit))
            {
                bool bitFoundInGroup = false;
                // is this bit part of a group
                for(quint16 group=0;
                    group<d->vecGroups.count() && !bitFoundInGroup;
                    group++)
                {
                    for(quint16 relay=0;
                        relay<d->vecGroups[group].arrSerializerRelayData.size() && !bitFoundInGroup;
                        relay++)
                    {
                        if(d->vecGroups[group].arrSerializerRelayData[relay].relayNum == ui16Bit)
                        {
                            bitFoundInGroup = true;
                            float newLoad = groupLoadCurrent[group];
                            if(d->logicalTargetMask.testBit(ui16Bit))
                                newLoad += d->vecGroups[group].arrSerializerRelayData[relay].supplyCurrentOn;
                            else
                                newLoad += d->vecGroups[group].arrSerializerRelayData[relay].supplyCurrentOff;
                            // Load is less or equal allowed sum
                            if(newLoad <= d->vecGroups[group].powerSupplyMaxCurrent)
                            {
                                // keep new sum
                                groupLoadCurrent[group] = newLoad;
                                // perform bit action
                                nextEnableMask.setBit(ui16Bit);
                                // set done for next
                                d->logicalDirtyMask.clearBit(ui16Bit);
                            }
                        }
                    }
                }
                // non group members -> transparent out
                if(!bitFoundInGroup)
                {
                    // perform bit action
                    nextEnableMask.setBit(ui16Bit);
                    // set done for next
                    d->logicalDirtyMask.clearBit(ui16Bit);
                }
            }
        }
        // start output
        d->lowRelayLayer->startSetMulti(nextEnableMask, d->logicalTargetMask);
        startedLowLayerTransaction = true;
    }
    return startedLowLayerTransaction;
}
