#ifndef QRelayBase_H
#define QRelayBase_H

#include <QObject>
#include <QBitArray>
#include <functional>
#include "relay_global.h"


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

    // and action
    virtual void startSetMulti(const QBitArray& logicalEnableMask,
                               const QBitArray& logicalSetMask,
                               bool bForce = false);
    void startSet(quint16 ui16BitNo,
                  bool bSet,
                  bool bForce = false);

    // query state
    virtual bool isBusy();

signals:
    void idle();

public slots:
    virtual void onLowLayerIdle();

protected:
    virtual void setupBaseBitmaps(quint16 ui16LogicalArrayInfoCount);

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
    void SetLowLayer(QRelayBase* lowRelayLayer);
    virtual void startSetMulti(const QBitArray& logicalEnableMask,
                               const QBitArray& logicalSetMask,
                               bool bForce = false);
    virtual const QBitArray& getLogicalRelayState();
protected:
    virtual bool process() = 0;   // true: this layer is still busy

public slots:
    virtual void onLowLayerIdle();

private slots:
    void onIdleTimer();

    Q_DECLARE_PRIVATE(QRelayUpperBase)
};

#endif // QRelayBase_H
