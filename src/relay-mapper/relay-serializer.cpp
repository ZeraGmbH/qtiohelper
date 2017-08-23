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

bool QRelaySerializer::addGroup(const TRelaySerializerGroup &group)
{
    Q_D(QRelaySerializer);
    int relay;
    bool checkOK = true;
    for(relay=0; relay<group.arrSerializerRelayData.size(); relay++)
    {
        // relays shall not be members of more than one group - check that
        quint16 relayNum = group.arrSerializerRelayData[relay].relayNum;
        bool switchOn = group.arrSerializerRelayData[relay].switchOn;
        if((switchOn && d->insertedRelaysOn.contains(relayNum)) ||
           (!switchOn && d->insertedRelaysOff.contains(relayNum)))
        {
            checkOK = false;
            qCritical() << "Relay" << relayNum << "is already member of a relay-serializer group. Group will be ignored!!";
        }
        // relay's current must be less or equal than power supply's current
        if(group.arrSerializerRelayData[relay].supplyCurrent > group.powerSupplyMaxCurrent)
        {
            checkOK = false;
            qCritical() << "Relay" << relayNum << ": current is larger than supply" << "Group will be ignored!!";
        }
    }
    if(checkOK)
    {
        d->vecGroups.append(group);
        for(relay=0; relay<group.arrSerializerRelayData.size(); relay++)
        {
            if(group.arrSerializerRelayData[relay].switchOn)
                d->insertedRelaysOn.insert(group.arrSerializerRelayData[relay].relayNum);
            else
                d->insertedRelaysOff.insert(group.arrSerializerRelayData[relay].relayNum);
        }
    }
    return checkOK;
}

void QRelaySerializer::appendSymetricRelay(QVector<TSerializerRelayData> &arrSerializerRelayData, quint16 relayNum, float supplyCurrent)
{
    arrSerializerRelayData.append(TSerializerRelayData(relayNum, supplyCurrent, true));
    arrSerializerRelayData.append(TSerializerRelayData(relayNum, supplyCurrent, false));
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
        QBitArray delayedForNextMask(getLogicalRelayCount());
        // To keep sequence set in groups we loop all group and check if their
        // relays are part of the transaction
        for(quint16 group=0; group<d->vecGroups.count(); group++)
        {
            for(quint16 relay=0; relay<d->vecGroups[group].arrSerializerRelayData.size();relay++)
            {
                struct TSerializerRelayData &relayData = d->vecGroups[group].arrSerializerRelayData[relay];
                // does relay number and desired on/off match
                if(d->logicalDirtyMask.testBit(relayData.relayNum) &&
                   d->logicalTargetMask.testBit(relayData.relayNum) == relayData.switchOn)
                {
                    float newLoad = groupLoadCurrent[group];
                    newLoad += d->vecGroups[group].arrSerializerRelayData[relay].supplyCurrent;
                    // Load is less or equal allowed sum
                    if(newLoad <= d->vecGroups[group].powerSupplyMaxCurrent)
                    {
                        // keep new sum
                        groupLoadCurrent[group] = newLoad;
                        // perform bit action
                        nextEnableMask.setBit(relayData.relayNum);
                        // set done
                        d->logicalDirtyMask.clearBit(relayData.relayNum);
                    }
                    // The bit/on-state matches but power supply load is exceeded
                    else
                    {
                        // we have to keep a reminder to avoid bit being switched in the
                        // transparent loop below
                        delayedForNextMask.setBit(relayData.relayNum);
                    }
                }
            }
        }
        for(quint16 ui16Bit=0; ui16Bit<getLogicalRelayCount(); ui16Bit++)
        {
            if(d->logicalDirtyMask.testBit(ui16Bit) && !delayedForNextMask.testBit(ui16Bit))
            {
                // perform bit action
                nextEnableMask.setBit(ui16Bit);
                // set done for next
                d->logicalDirtyMask.clearBit(ui16Bit);
            }
        }
        // start output
        d->lowRelayLayer->startSetMulti(nextEnableMask, d->logicalTargetMask);
        startedLowLayerTransaction = true;
    }
    return startedLowLayerTransaction;
}
