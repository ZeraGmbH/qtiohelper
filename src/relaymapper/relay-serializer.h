#ifndef QRelaySerializer_H
#define QRelaySerializer_H

#include "relay-base.h"
#include <QVector>

struct TSerializerRelayData
{
    TSerializerRelayData()
    {
        relayNum = 0;
        supplyCurrent = 0.0;
        switchOn = false;
    }
    TSerializerRelayData(quint16 relayNum_, float supplyCurrent_, bool switchOn_)
    {
        relayNum = relayNum_;
        supplyCurrent = supplyCurrent_;
        switchOn = switchOn_;
    }
    quint16 relayNum;
    float supplyCurrent;
    bool switchOn;
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
    QRelaySerializer(QObject *parent = nullptr);

    bool addGroup(const TRelaySerializerGroup &group);
    void appendSymetricRelay(QVector<TSerializerRelayData> &arrSerializerRelayData, quint16 relayNum, float supplyCurrent);

protected:
    virtual void setupBaseBitmaps(quint16 ui16LogicalArrayInfoCount);
    virtual bool process();

private:
    Q_DECLARE_PRIVATE(QRelaySerializer)
};

#endif // QRelaySerializer_H
