#ifndef QActuaSense_H
#define QActuaSense_H

#include <QObject>
#include <QTimer>
#include <functional>
#include "actuasense_global.h"

class QActuaSensePrivate;
class QActuaSenseIOParams;

// Callback function for lower layer transaction
//  * Only bits which are set in EnableMask are handled - other bits in SetMask are ignored
typedef std::function<void(const QBitArray& EnableMask, const QBitArray& SetMask)> ActuaSenseStartLowLayerSwitchFunction;

class QTACTUASENSESHARED_EXPORT QActuaSense : public QObject
{
    Q_OBJECT
public:
    QActuaSense(QObject *parent = NULL);

    // setup
    void setInBitMask(const QBitArray *pInBitArr);
    void setStartLowLayerCallback(ActuaSenseStartLowLayerSwitchFunction pFunc);
    void addAtomicIn(int iActionID, int iInBitNum);
    void addAtomicOut(int iActionID, int iOutBitNum);
    // we can override the automatic biitmap size alignment implemented in
    // addAtomic.. e.g if the mask sized must be something dividable by 8
    void setBitMaskSize(int iSize);

    // setup demo
    // note iActionID == -1 all currently found
    void setupDemo(bool bDemoMode, int iActionID = -1, QBitArray *pDemoBitArrIn = NULL, int iDemoBitNumIn=-1);
    void setDemoError(bool bDemoError, int iActionID);

    // optional internal in poll timer - can be handled externally
    void startInternalInputPoll(int milliSeconds);
    void stopInternalInputPoll();

    // we handle so called MultiActions (multiple pin transitions at one time)
    void openMultiAction();
    void startOutSet(int iActionID, bool bStateOut);
    void startInObserve(int iActionID, bool bStateInDesired, int iTimeoutMs,
                        QString strOK = QString(), QString strErr = QString(), QString strLongTermErr = QString());
    void closeMultiAction();

    // convenience
    bool readInputState(int iActionID);
    void removeFromLongObserv(int iActionID); // note iActionID == -1 all

signals:
    void multiActionFinished(bool bError);
    void longTermObservationError();

public slots:
    void onPollTimer();

private:
    QTimer m_IoPollTimer;

    QActuaSensePrivate *d_ptr;
    Q_DECLARE_PRIVATE(QActuaSense)
};

#endif // QActuaSense_H
