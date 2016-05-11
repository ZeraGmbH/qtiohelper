#ifndef QActuaSense_P_H
#define QActuaSense_P_H

#include <QString>
#include <QSet>
#include <QHash>
#include <QBitArray>
#include <QElapsedTimer>
#include "actuasense.h"

class QActuaSenseIOParams : public QObject
{
    Q_OBJECT
public:
    QActuaSenseIOParams(QObject* pParent);
    void setIn(int iInBitNum);
    void setOut(int iOutBitNum);
    void setDemoInfo(bool bDemoMode, QBitArray *pDemoBitArrIn = NULL, int iDemoBitNumIn=-1);

    int m_iOutBitNum;
    int m_iInBitNum;

    bool m_bDemoMode;
    int m_iDemoBitNumIn;
    QBitArray *m_pDemoBitArrIn;
    bool m_bDemoError;

signals:

public slots:
    // after called with true all further readInputStates will return with state unequal desired
    void onDemoSetStateError(bool bError);
};

class QActuaSenseAction : public QObject
{
    Q_OBJECT
public:
    QActuaSenseAction(QActuaSenseIOParams* pAtomicIO);

    QActuaSenseIOParams* m_pIOParams;
    int m_iTimeoutMs;
    int m_iMsSinceLastSet;
    bool m_bInStateDesired;

    QString m_strOK;
    QString m_strErr;
    QString m_strLongTermErr;
};

enum enActionPoolType
{
    ACTION_POOL_TYPE_INACTIVE = 0,
    ACTION_POOL_TYPE_ACTIVE,
    ACTION_POOL_TYPE_LONG_OBSERVE,

    ACTION_POOL_TYPE_COUNT
};

class QActuaSenseActionPointerArray
{
public:
    QActuaSenseActionPointerArray(QActuaSenseAction* pAction);
    ~QActuaSenseActionPointerArray();

    QActuaSenseAction* moveActionTo(enum enActionPoolType eNewType);
    QActuaSenseAction* find();

    QActuaSenseAction* m_arrpAction[ACTION_POOL_TYPE_COUNT];
};

typedef QHash<int, QActuaSenseIOParams*> QActuaSenseIOParamsIntHash;
typedef QHash<int, QActuaSenseActionPointerArray*> QActuaSenseActionPointerArrayIntHash;

class QActuaSensePrivate
{
public:
    QActuaSensePrivate();
    virtual ~QActuaSensePrivate();

    bool hasReachedDestinationState(QActuaSenseAction *pAction);
    bool hasTimedOut(QActuaSenseAction *pAction);
    bool readInputState(QActuaSenseAction *pAction);

    QBitArray m_OutEnableBitArr;
    QBitArray m_OutSetBitArr;
    const QBitArray *m_pInBitArr;
    ActuaSenseStartLowLayerSwitchFunction m_pLowLayerStartFunc;

    QActuaSenseIOParamsIntHash m_PoolIOData;

    bool m_bInAddingActions;

    QActuaSenseActionPointerArrayIntHash m_PoolActionsArray;

    QElapsedTimer m_TimerElapsedLastPoll;
};

#endif // QActuaSense_P_H

