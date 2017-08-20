#ifndef QRelaySerializer_P_H
#define QRelaySerializer_P_H

#include "relay-base_p.h"
#include "relay-serializer.h"
#include <QSet>
#include <QList>

class QRelaySerializerPrivate : public QRelayUpperBasePrivate
{
public:
    QRelaySerializerPrivate();
    virtual ~QRelaySerializerPrivate();

    QList<TRelaySerializerGroup> listGroups;
    QSet<quint16> insertedRelays;
};


#endif // QRelaySerializer_P_H

