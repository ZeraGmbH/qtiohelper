#ifndef QRelaySerializer_H
#define QRelaySerializer_H

#include "relay-base.h"
#include <QVector>

struct TSerializerRelayData
{
    TSerializerRelayData()
    {
        relayNum = 0;
        supplyCurrentOn = 0.0;
        supplyCurrentOff = 0.0;
    }
    TSerializerRelayData(quint16 relayNum_, float supplyCurrentOn_, float supplyCurrentOff_)
    {
        relayNum = relayNum_;
        supplyCurrentOn = supplyCurrentOn_;
        supplyCurrentOff = supplyCurrentOff_;
    }
    quint16 relayNum;
    float supplyCurrentOn;
    float supplyCurrentOff;
};

struct TRelaySerializerGroup
{
    TRelaySerializerGroup()
    {
        powerSupplyMaxCurrent = 0.0;
    }
    TRelaySerializerGroup(float powerSupplyMaxCurrent_, QVector<TSerializerRelayData> arrSerializerRelayData_ )
    {
        powerSupplyMaxCurrent = powerSupplyMaxCurrent_;
        arrSerializerRelayData = arrSerializerRelayData_;
    }
    float powerSupplyMaxCurrent;
    QVector<TSerializerRelayData> arrSerializerRelayData;
};

class QRelaySerializerPrivate;

class QTRELAYSSHARED_EXPORT QRelaySerializer : public QRelayUpperBase
{
    Q_OBJECT
public:
    QRelaySerializer(QObject *parent = Q_NULLPTR);

    bool AddGroup(const TRelaySerializerGroup &group);

protected:
    virtual void setupBaseBitmaps(quint16 ui16LogicalArrayInfoCount);
    virtual bool process();

private:
    Q_DECLARE_PRIVATE(QRelaySerializer)
};

#endif // QRelaySerializer_H
