#ifndef QRelayBase_H
#define QRelayBase_H

#include <QObject>
#include <QBitArray>
#include <functional>
#include "relaymapper_global.h"


class QRelayBasePrivate;

class QTRELAYSSHARED_EXPORT QRelayBase : public QObject
{
    Q_OBJECT
public:
    QRelayBase(QObject *parent, QRelayBasePrivate *dp);
    virtual ~QRelayBase();

    // setup getter
    quint16 getLogicalRelayCount();
    virtual const QBitArray& getLogicalRelayState() = 0;

    // the startSet functions collect data to be performed on next
    // idle of machine (0ms timer).
    virtual void startSetMulti(
            const QBitArray& logicalEnableMask, // only bits set here are performed
            const QBitArray& logicalSetMask,
            bool bForce = false); // see QRelayUpperBase and QRelayMapper for more details
    void startSet(
            quint16 ui16BitNo,
            bool bSet,
            bool bForce = false);

    // query state
    virtual bool isBusy();

signals:
    // notification for all jobs done
    void idle();

public slots:
    // an upper layer should connect to lower layer's idle signal
    virtual void onLowLayerIdle();

protected:
    virtual void setupBaseBitmaps(quint16 ui16LogicalArrayInfoCount);
    bool startNextTransaction();

    QRelayBasePrivate *d_ptr;
    Q_DECLARE_PRIVATE(QRelayBase)
};

class QRelayUpperBasePrivate;

// base class for all layers which have a relay object as lower layer
class QTRELAYSSHARED_EXPORT QRelayUpperBase : public QRelayBase
{
    Q_OBJECT
public:
    QRelayUpperBase(QObject *parent, QRelayUpperBasePrivate *dp);
    void setLowLayer(QRelayBase* lowRelayLayer);
    virtual void startSetMulti(
            const QBitArray& logicalEnableMask,
            const QBitArray& logicalSetMask,
            bool bForce = false); // =true: This layer passed input transparent
                                  // to low layer connected
    virtual const QBitArray& getLogicalRelayState();
    virtual bool isBusy();
protected:
    virtual bool process() = 0;   // return true: this layer is still busy

public slots:
    virtual void onLowLayerIdle();

private slots:
    void onIdleTimer();

    Q_DECLARE_PRIVATE(QRelayUpperBase)
};

#endif // QRelayBase_H
