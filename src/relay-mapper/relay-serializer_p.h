#ifndef QRelaySerializer_P_H
#define QRelaySerializer_P_H

#include "relay-base_p.h"
#include "relay-serializer.h"
#include <QSet>
#include <QVector>

class QRelaySerializerPrivate : public QRelayUpperBasePrivate
{
public:
    QRelaySerializerPrivate();
    virtual ~QRelaySerializerPrivate();

    QVector<TRelaySerializerGroup> vecGroups;
    QSet<quint16> insertedRelaysOn;
    QSet<quint16> insertedRelaysOff;
};


#endif // QRelaySerializer_P_H

