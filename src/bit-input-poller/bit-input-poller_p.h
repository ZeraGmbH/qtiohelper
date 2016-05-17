#ifndef QBitInputPoller_P_H
#define QBitInputPoller_P_H

#include <QObject>
#include <QBitArray>
#include <QTimer>
#include "bit-input-poller.h"

class QBitInputPollerPrivate
{
public:
    QBitInputPollerPrivate();
    virtual ~QBitInputPollerPrivate();

    QBitArray m_BitMaskInput;
    QBitArray m_BitMaskInvert;
    StartBitReadFunction m_pStartBitReadFunction;

    QTimer m_PollTimer;
};


#endif // QBitInputPoller_P_H

