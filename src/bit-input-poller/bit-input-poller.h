#ifndef QBitInputPoller_H
#define QBitInputPoller_H

#include <QObject>
#include <QTimer>
#include <QBitArray>
#include <functional>
#include "bit-input-poller_global.h"

class QBitInputPollerPrivate;

// Callback function for bit read functuion
//   * return value true signals non-blocking or we'll wait for onBitMaskReadFinish
typedef std::function<bool(QBitArray* pEnableMask)> StartBitReadFunction;

class QTBITINPUTPOLLERSHARED_EXPORT QBitInputPoller : public QObject
{
    Q_OBJECT
public:
    QBitInputPoller(QObject *parent = NULL);
    void setupInputMask(int iCountBits);
    const QBitArray* getInputBitmask();
    void setStartBitReadFunction(StartBitReadFunction pFunc);

    void startPoll(int iMilliSecCycle);
    void stopPoll();

signals:
    void bitmaskUpdated();

public slots:
    void onBitMaskReadFinish();     // non-blocking handler signals should be connected here

private slots:
    void onPollTimer();

private:
    QTimer m_PollTimer;

    QBitInputPollerPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QBitInputPoller)
};

#endif // QBitInputPoller_H
