#ifndef QBitInputPoller_P_H
#define QBitInputPoller_P_H

#include <QObject>
#include <QBitArray>
#include "bit-input-poller.h"

class QBitInputPollerPrivate
{
public:
    QBitInputPollerPrivate();
    virtual ~QBitInputPollerPrivate();

    QBitArray m_BitMaskInput;
    QBitArray m_BitMaskInvert;
    StartBitReadFunction m_pStartBitReadFunction;
};


#endif // QBitInputPoller_P_H

