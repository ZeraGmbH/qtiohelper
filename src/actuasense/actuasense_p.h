#ifndef QActuaSense_P_H
#define QActuaSense_P_H

#include <QString>
#include <QSet>
#include <QHash>
#include <QBitArray>
#include <QTimer>
#include <QElapsedTimer>
#include "actuasense.h"

enum enActionState
{
    ACTION_STATE_INACTIVE = 0,
    ACTION_STATE_ACTIVE,
    ACTION_STATE_LONG_OBSERVE,
    ACTION_STATE_LONG_OBSERVE_ERR
};

class QActuaSenseIOData : public QObject
{
    Q_OBJECT
public:
    QActuaSenseIOData(QObject* pParent);
    void setDemoInfo(bool bDemoMode, QBitArray *pDemoBitArrIn = nullptr, int iDemoBitNumIn=-1);

    int m_iOutBitNum;
    int m_iInBitNum;

    bool m_bDemoMode;
    int m_iDemoBitNumIn;
    QBitArray *m_pDemoBitArrIn;
    bool m_bDemoError;

    int m_iTimeoutMs;
    int m_iMsSinceLastSet;
    bool m_bInStateDesired;

    QString m_strOK;
    QString m_strErr;
    QString m_strLongTermErr;

    enum enActionState m_eActionState;
};

typedef QHash<int, QActuaSenseIOData*> QActuaSenseIODataIntHash;

class QActuaSensePrivate
{
    Q_DECLARE_PUBLIC(QActuaSense)
public:
    QActuaSensePrivate(QActuaSense* pPublic);
    virtual ~QActuaSensePrivate();

    void openMultiAction();
    void startOutSet(int iActionID, bool bStateOut);
    void startInObserve(int iActionID, bool bStateInDesired, int iTimeoutMs,
                        QString strOK, QString strErr, QString strLongTermErr);
    void closeMultiAction();

    bool readInputState(int iActionID);
    bool readInputState(QActuaSenseIOData *pActionData);
    void removeFromLongObserv(int iActionID); // note iActionID == -1 all
    QActuaSenseIOData* findOrCreateIOParam(int iActionID, QObject *pParent);

    bool hasReachedDestinationState(QActuaSenseIOData *pActionData);
    bool hasTimedOut(QActuaSenseIOData *pActionData);

    bool getLongTermStatus(QString &strErr, bool bNotifyOnce = true);

    void onPollTimer();

private:
    QBitArray m_OutEnableBitArr;
    QBitArray m_OutSetBitArr;
    const QBitArray *m_pInBitArr;
    ActuaSenseStartLowLayerSwitchFunction m_pLowLayerStartFunc;

    bool m_bInAddingActions;
    QActuaSenseIODataIntHash m_PoolIOData;

    QElapsedTimer m_TimerElapsedLastPoll;
    QTimer m_IoPollTimer;

    QActuaSense *q_ptr;
};

#endif // QActuaSense_P_H

