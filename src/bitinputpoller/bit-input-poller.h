#ifndef QBITINPUTPOLLER_H 
#define QBITINPUTPOLLER_H

#include <QObject>
#include <QBitArray>
#include <functional>
#include "bitinputpoller_global.h"

class QBitInputPollerPrivate;

// Callback function for bit read functuion
//   * return value true signals non-blocking or we'll wait for onBitMaskReadFinish
//  * NON-BLOCKING:
//      -return value true
//      -low layer fills pInMask and gives notification on onBitMaskReadFinish
//  * BLOCKING:
//      -return value false - onBitMaskReadFinish is never called

typedef std::function<bool(QBitArray* pInMask)> StartBitReadFunction;

class QTBITINPUTPOLLERSHARED_EXPORT QBitInputPoller : public QObject
{
    Q_OBJECT
public:
    QBitInputPoller(QObject *parent = nullptr);
    void setupInputMask(int iCountBits, const QBitArray& mask = QBitArray());
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
    QBitInputPollerPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QBitInputPoller)
};

#endif // QBITINPUTPOLLER_H
