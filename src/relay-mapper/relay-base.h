#ifndef QRelayBase_H
#define QRelayBase_H

#include <QObject>
#include <QBitArray>
#include <functional>
#include "relay_global.h"

// Callback function for asking if lower layer is busy
typedef std::function<bool()> RelayQueryLowLayerBusy;

// Callback function for switching lower layer
//  * Only bits which are set in EnableMask are handled - other bits in SetMask are ignored
//  * NON-BLOCKING:
//      -return value true (=busy)
//      -low layer signal idle should be connected to onLowLayerIdle
//  * BLOCKING:
//      -return value false - onLowLayerIdle within current layer itself
typedef std::function<bool(const QBitArray& EnableMask, const QBitArray& SetMask, const QObject *pSignalHandler)> RelayStartLowLayerSwitchFunction;

class QRelayBasePrivate;

class QTRELAYSSHARED_EXPORT QRelayBase : public QObject
{
    Q_OBJECT
public:
    QRelayBase(QObject *parent, QRelayBasePrivate *dp);
    virtual ~QRelayBase();

    // setup busy callback
    void setupCallbackLowLayerBusy(RelayQueryLowLayerBusy callback);

    // setup get
    quint16 getLogicalRelayCount();
    const QBitArray& getLogicalRelayState();

    // and action
    void startSetMulti(const QBitArray& logicalEnableMask,  // our implementation should work for all -> not virtual
                       const QBitArray& logicalSetMask,
                       bool bForce = false);
    virtual void startSet(quint16 ui16BitNo,
                          bool bSet,
                          bool bForce = false);

    // query state
    virtual bool isBusy();

signals:
    void idle();

public slots:
    virtual void onLowLayerIdle();

protected:
    virtual void idleCleanup() {}

    void setup(quint16 ui16LogicalArrayInfoCount, RelayStartLowLayerSwitchFunction CallbackStartLowLayerSwitch);

    QRelayBasePrivate *d_ptr;
    Q_DECLARE_PRIVATE(QRelayBase)
};

#endif // QRelayBase_H
