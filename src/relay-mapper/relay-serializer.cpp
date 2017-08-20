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
        d->listGroups.append(group);
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
    bool idleOut = false;

    return !idleOut;
}
