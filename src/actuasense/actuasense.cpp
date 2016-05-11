#include "actuasense.h"
#include "actuasense_p.h"

// ************************** QActuaSenseIOParams
QActuaSenseIOParams::QActuaSenseIOParams(QObject* pParent) : QObject(pParent)
{
    m_iInBitNum = -1;
    m_iOutBitNum = -1;
    m_bDemoMode = false;
    m_iDemoBitNumIn = -1;
    m_pDemoBitArrIn = NULL;
    m_bDemoError = false;
}

void QActuaSenseIOParams::setIn(int iInBitNum)
{
    m_iInBitNum = iInBitNum;
}

void QActuaSenseIOParams::setOut(int iOutBitNum)
{
    m_iOutBitNum = iOutBitNum;
}

void QActuaSenseIOParams::setDemoInfo(bool bDemoMode, QBitArray *pDemoBitArrIn, int iDemoBitNumIn)
{
    m_bDemoMode = bDemoMode;
    m_iDemoBitNumIn = iDemoBitNumIn;
    m_pDemoBitArrIn = pDemoBitArrIn;
}

void QActuaSenseIOParams::onDemoSetStateError(bool bError)
{
    m_bDemoError = bError;
}

// ************************** QActuaSenseAction
QActuaSenseAction::QActuaSenseAction(QActuaSenseIOParams* pAtomicIO) : QObject(pAtomicIO)
{
    m_pIOParams = m_pIOParams;
    m_iTimeoutMs = 0;
    m_bInStateDesired = false;
    m_iMsSinceLastSet = 0;
}

// ************************** QActuaSensePrivate

QActuaSensePrivate::QActuaSensePrivate()
{
    m_pInBitArr = NULL;
    m_pLowLayerStartFunc = NULL;
}

QActuaSensePrivate::~QActuaSensePrivate()
{
    for(QActuaSenseIOParamsIntHash::iterator iter=m_PoolIOData.begin(); iter!=m_PoolIOData.end(); iter++)
        delete iter.value();
    m_PoolIOData.clear();
}

QActuaSenseAction *QActuaSensePrivate::MoveActionTo(int iActionID, enum enActionPoolType eType)
{
    QActuaSenseAction *pAction = NULL;
    // find & remove action from other pools
    for(int iType=0; iType<ACTION_POOL_TYPE_COUNT; iType++)
    {
        if(iType != eType)
        {
            QActuaSenseActionIntHash::iterator iterActions = m_PoolActionsArray[iType].find(iActionID);
            if(iterActions != m_PoolActionsArray[iType].end())
            {
                pAction = iterActions.value();
                m_PoolActionsArray[iType].erase(iterActions);
                break;
            }
        }
    }
    if(pAction)
        // add to active actions
        m_PoolActionsArray[eType][iActionID] = pAction;
    // pAction == NULL -> action is already in desired pool
    else
    {
        QActuaSenseActionIntHash::iterator iterActions = m_PoolActionsArray[eType].find(iActionID);
        if(iterActions != m_PoolActionsArray[eType].end())
            pAction = iterActions.value();
        else
            qFatal("QActuaSensePrivate::MoveActionTo: No action with ID %i found!", iActionID);
    }
    return pAction;
}

// ************************** local helpers
bool readInputState(QActuaSenseAction *pAction, const QBitArray *pInBitArr)
{
    bool bInState = false;
    if(!pAction->m_pIOParams->m_bDemoMode)
        bInState = pInBitArr->at(pAction->m_pIOParams->m_iInBitNum);
    // demo mode
    else
    {
        // full input demo
        if(pAction->m_pIOParams->m_pDemoBitArrIn && pAction->m_pIOParams->m_iDemoBitNumIn >=0)
            bInState = pAction->m_pIOParams->m_pDemoBitArrIn->at(pAction->m_pIOParams->m_iDemoBitNumIn);
        // demo error
        else if(pAction->m_pIOParams->m_bDemoError)
            bInState = !pAction->m_bInStateDesired;
        // standard demo
        else
        {
            // standard demo reaches desired state at half timeout
            if(pAction->m_iMsSinceLastSet >= pAction->m_iTimeoutMs/2 )
                bInState = pAction->m_bInStateDesired;
            else
                bInState = !pAction->m_bInStateDesired;
        }
    }
    return bInState;
}

bool hasTimedOut(QActuaSenseAction *pAction)
{
    return pAction->m_iMsSinceLastSet >= pAction->m_iTimeoutMs;
}

bool hasReachedDestinationState(QActuaSenseAction *pAction, const QBitArray *pInBitArr)
{
    // In case no input pin was set: use timeout
    if(pAction->m_pIOParams->m_iInBitNum < 0)
        return hasTimedOut(pAction);
    return pAction->m_bInStateDesired == readInputState(pAction, pInBitArr);
}

// ************************** QActuaSense

QActuaSense::QActuaSense(QObject *parent) :
    QObject(parent),
    d_ptr(new QActuaSensePrivate())
{
}

void QActuaSense::setInBitMask(const QBitArray *pInBitArr)
{
    Q_D(QActuaSense);
    d->m_pInBitArr = pInBitArr;
}

void QActuaSense::setStartLowLayerCallback(ActuaSenseStartLowLayerSwitchFunction pFunc)
{
    Q_D(QActuaSense);
    d->m_pLowLayerStartFunc = pFunc;
}

void QActuaSense::addAtomicIn(int iActionID, int iInBitNum)
{
    Q_D(QActuaSense);
    QActuaSenseIOParamsIntHash::iterator iter = d->m_PoolIOData.find(iActionID);
    QActuaSenseIOParams* pIOParams;
    if(iter == d->m_PoolIOData.end())
    {
        // create a new parameter and add to inactive actions
        pIOParams = new QActuaSenseIOParams(this);
        d->m_PoolIOData[iActionID] = pIOParams;
        d->m_PoolActionsArray[ACTION_POOL_TYPE_INACTIVE][iActionID] = new QActuaSenseAction(pIOParams);
    }
    else
        pIOParams = iter.value();
    pIOParams->setIn(iInBitNum);
}

void QActuaSense::addAtomicOut(int iActionID, int iOutBitNum)
{
    Q_D(QActuaSense);
    QActuaSenseIOParamsIntHash::iterator iter = d->m_PoolIOData.find(iActionID);
    QActuaSenseIOParams* pIOParams;
    if(iter == d->m_PoolIOData.end())
    {
        // create a new parameter
        pIOParams = new QActuaSenseIOParams(this);
        d->m_PoolIOData[iActionID] = pIOParams;
    }
    else
        pIOParams = iter.value();
    pIOParams->setOut(iOutBitNum);
    if(iOutBitNum+1 > d->m_OutSetBitArr.count())
    {
        d->m_OutEnableBitArr.resize(iOutBitNum+1);
        d->m_OutSetBitArr.resize(iOutBitNum+1);
    }
}

void QActuaSense::setupDemo(bool bDemoMode, int iActionID, QBitArray *pDemoBitArrIn, int iDemoBitNumIn)
{
    Q_D(QActuaSense);
    QActuaSenseIOParamsIntHash::iterator iter;
    QActuaSenseIOParams* pIOParams;
    // one
    if(iActionID != -1)
    {
        iter = d->m_PoolIOData.find(iActionID);
        if(iter != d->m_PoolIOData.end())
        {
            pIOParams = iter.value();
            pIOParams->m_bDemoMode = bDemoMode;
            pIOParams->m_pDemoBitArrIn = pDemoBitArrIn;
            pIOParams->m_iDemoBitNumIn = iDemoBitNumIn;
        }
    }
    // all
    else
    {
        for(iter = d->m_PoolIOData.begin(); iter!=d->m_PoolIOData.end(); iter++)
        {
            pIOParams = iter.value();
            pIOParams->m_bDemoMode = bDemoMode;
            pIOParams->m_pDemoBitArrIn = pDemoBitArrIn;
            pIOParams->m_iDemoBitNumIn = iDemoBitNumIn;
        }
    }
}

void QActuaSense::setDemoError(bool bDemoError, int iActionID)
{
    Q_D(QActuaSense);
    QActuaSenseIOParamsIntHash::iterator iter;
    QActuaSenseIOParams* pIOParams;
    // one
    if(iActionID != -1)
    {
        iter = d->m_PoolIOData.find(iActionID);
        if(iter != d->m_PoolIOData.end())
        {
            pIOParams = iter.value();
            pIOParams->m_bDemoError = bDemoError;
        }
    }
    // all
    else
    {
        for(iter = d->m_PoolIOData.begin(); iter!=d->m_PoolIOData.end(); iter++)
        {
            pIOParams = iter.value();
            pIOParams->m_bDemoError = bDemoError;
        }
    }
}

void QActuaSense::startInternalInputPoll(int milliSeconds)
{
    m_IoPollTimer.setSingleShot(false);
    m_IoPollTimer.start(milliSeconds);
    connect(&m_IoPollTimer,&QTimer::timeout, this, &QActuaSense::onPollTimer);
}

void QActuaSense::stopInternalInputPoll()
{
    m_IoPollTimer.stop();
    disconnect(&m_IoPollTimer,&QTimer::timeout, this, &QActuaSense::onPollTimer);
}

void QActuaSense::openMultiAction()
{
    Q_D(QActuaSense);
    // if the last multi action failed we have entries left in active
    // move them to inactive
    for(QActuaSenseActionIntHash::iterator iter = d->m_PoolActionsArray[ACTION_POOL_TYPE_ACTIVE].begin();
        iter != d->m_PoolActionsArray[ACTION_POOL_TYPE_ACTIVE].end();
        iter++)
        d->m_PoolActionsArray[ACTION_POOL_TYPE_INACTIVE][iter.key()] = iter.value();
    d->m_PoolActionsArray[ACTION_POOL_TYPE_ACTIVE].clear();
    d->m_bInAddingActions = true;
}

void QActuaSense::outSetStart(int iActionID, bool bStateOut)
{
    Q_D(QActuaSense);
    QActuaSenseIOParamsIntHash::iterator iter = d->m_PoolIOData.find(iActionID);
    if(iter != d->m_PoolIOData.end())
    {
        QActuaSenseIOParams* pIOParams = iter.value();
        if(pIOParams->m_iOutBitNum>=0)
        {
            d->m_OutEnableBitArr.setBit(pIOParams->m_iOutBitNum);
            d->m_OutSetBitArr.setBit(pIOParams->m_iOutBitNum, bStateOut);
            // if we are not in a multi action we transfer now otherwise keep set data
            // and transfer in closeMultiAction
            if(!d->m_bInAddingActions && d->m_pLowLayerStartFunc)
            {
                // just fire and forget
                d->m_pLowLayerStartFunc(d->m_OutEnableBitArr, d->m_OutSetBitArr);
                // avoid setting this bit in next transaction
                d->m_OutEnableBitArr.setBit(pIOParams->m_iOutBitNum, false);
            }
        }
        else
            qWarning("outSetStart: iActionId %i no out information set!", iActionID);
    }
    else
        qWarning("outSetStart: iActionId %i not found!", iActionID);
}

void QActuaSense::inStartObserve(int iActionID,
                                 bool bStateInDesired,
                                 int iTimeoutMs,
                                 QString strOK, QString strErr, QString strLongTermErr)
{
    Q_D(QActuaSense);
    QActuaSenseIOParamsIntHash::iterator iter = d->m_PoolIOData.find(iActionID);
    if(iter != d->m_PoolIOData.end())
    {
        QActuaSenseAction *pAction = NULL;
        // We are inside a transaction -> active
        if(d->m_bInAddingActions)
            pAction = d->MoveActionTo(iActionID, ACTION_POOL_TYPE_ACTIVE);
        // outside transaction
        else
        {
            // move to long term if error message is set
            if(!strLongTermErr.isEmpty())
                pAction = d->MoveActionTo(iActionID, ACTION_POOL_TYPE_LONG_OBSERVE);
            // move to inactive
            else
                pAction = d->MoveActionTo(iActionID, ACTION_POOL_TYPE_INACTIVE);
        }
        if(pAction)
        {
            pAction->m_iTimeoutMs = iTimeoutMs;
            pAction->m_iMsSinceLastSet = 0;
            pAction->m_bInStateDesired = bStateInDesired;
            pAction->m_strOK = strOK;
            pAction->m_strErr = strErr;
            pAction->m_strLongTermErr = strLongTermErr;
        }
    }
    else
        qWarning("inStartObserve: iActionId %i not found!", iActionID);
}

void QActuaSense::closeMultiAction()
{
    Q_D(QActuaSense);
    d->m_bInAddingActions = false;
    if(d->m_pLowLayerStartFunc && d->m_OutEnableBitArr.count(true) != 0)
    {
        // start low layer transaction
        // note: we don't care if low layer call does block or not - MultiActionFinished by
        // input conditions polled
        d->m_pLowLayerStartFunc(d->m_OutEnableBitArr, d->m_OutSetBitArr);
        // reset flags to avoid setting in next transaction
        d->m_OutEnableBitArr.fill(false);
    }
}

bool QActuaSense::readInputState(int iActionID)
{
    Q_D(QActuaSense);
    bool bInState = false;
    QActuaSenseIOParamsIntHash::iterator iter = d->m_PoolIOData.find(iActionID);
    if(iter != d->m_PoolIOData.end())
    {

        QActuaSenseIOParams* pIOParams = iter.value();
        if(pIOParams->m_iInBitNum >= 0)
        {
            // The following is a bit weird: One could ask 'why
            // does ::readInputState require an action as parameter?'
            // Answer: to support smooth demo behaviour
            // So we need to find where the action is found
            QActuaSenseAction *pAction = NULL;
            for(int iType=0; iType<ACTION_POOL_TYPE_COUNT; iType++)
            {
                QActuaSenseActionIntHash::iterator iterActions = d->m_PoolActionsArray[iType].find(iActionID);
                if(iterActions != d->m_PoolActionsArray[iType].end())
                {
                    pAction = iterActions.value();
                    break;
                }
            }
            bInState = ::readInputState(pAction, d->m_pInBitArr);
        }
        else
            qWarning("No input set for action %i", iActionID);
    }
    else
        qWarning("readInputState: iActionId %i not found!", iActionID);
    return bInState;
}

void QActuaSense::onPollTimer()
{
    Q_D(QActuaSense);
    // active actions
    QActuaSenseActionIntHash* pActionPool = &d->m_PoolActionsArray[ACTION_POOL_TYPE_ACTIVE];
    QActuaSenseActionIntHash::iterator iter;
    QActuaSenseActionIntHash::iterator iterRemove;
    QActuaSenseAction* pAction = NULL;
    if(!d->m_bInAddingActions && !pActionPool->isEmpty())
    {
        int iTimeSinceLastPoll = d->m_TimerElapsedLastPoll.restart();
        iter = pActionPool->begin();
        bool bActionError = false;
        while(iter != pActionPool->end())
        {
            pAction = iter.value();
            pAction->m_iMsSinceLastSet += iTimeSinceLastPoll;
            iterRemove = pActionPool->end();
            // action finished?
            if(hasReachedDestinationState(pAction, d->m_pInBitArr))
            {
                // Drop OK string
                if(!pAction->m_strOK.isEmpty())
                    qInfo(pAction->m_strOK.toLatin1());
                // keep for deletion
                iterRemove = iter;
                // pass to long term observation
                if(!pAction->m_strLongTermErr.isEmpty())
                    d->m_PoolActionsArray[ACTION_POOL_TYPE_LONG_OBSERVE][iter.key()] = pAction;
                // pass to inactive
                else
                    d->m_PoolActionsArray[ACTION_POOL_TYPE_INACTIVE][iter.key()] = pAction;
            }
            // action timed out?
            else if(hasTimedOut(pAction))
            {
                // treat as error only for error string set
                if(!pAction->m_strErr.isEmpty())
                {
                    bActionError = true;
                    qCritical(pAction->m_strErr.toLatin1());
                }
                iterRemove = iter;
                // pass to inactive
                d->m_PoolActionsArray[ACTION_POOL_TYPE_INACTIVE][iter.key()] = pAction;
            }
            // remove from active (was copied above)?
            if(iterRemove != d->m_PoolActionsArray[ACTION_POOL_TYPE_ACTIVE].end())
                iter = d->m_PoolActionsArray[ACTION_POOL_TYPE_ACTIVE].erase(iterRemove);
            else
                iter++;
        }
        // in case of error: move all remaining active actions -> inactive to avoid multiple fire
        if(bActionError)
        {
            for(iterRemove = d->m_PoolActionsArray[ACTION_POOL_TYPE_ACTIVE].begin();
                iterRemove != d->m_PoolActionsArray[ACTION_POOL_TYPE_ACTIVE].end();
                iterRemove++)
                d->m_PoolActionsArray[ACTION_POOL_TYPE_INACTIVE][iterRemove.key()] = iterRemove.value();
            d->m_PoolActionsArray[ACTION_POOL_TYPE_ACTIVE].clear();
        }
        // active actions finished -> notification
        if(d->m_PoolActionsArray[ACTION_POOL_TYPE_ACTIVE].isEmpty())
            emit MultiActionFinished(bActionError);
    }
    // long term monitoring
    if(!d->m_PoolActionsArray[ACTION_POOL_TYPE_LONG_OBSERVE].isEmpty())
    {
        bool bLongTermObservationFailed = false;
        iter = d->m_PoolActionsArray[ACTION_POOL_TYPE_LONG_OBSERVE].begin();
        while(iter != d->m_PoolActionsArray[ACTION_POOL_TYPE_LONG_OBSERVE].end())
        {
            pAction = iter.value();
            iterRemove = d->m_PoolActionsArray[ACTION_POOL_TYPE_LONG_OBSERVE].end();
            // Lost it's destination state?
            if(!hasReachedDestinationState(pAction, d->m_pInBitArr))
            {
                if(!pAction->m_strLongTermErr.isEmpty())
                    qCritical(pAction->m_strLongTermErr.toLatin1());
                iterRemove = iter;
                bLongTermObservationFailed = true;
                // pass to inactive
                d->m_PoolActionsArray[ACTION_POOL_TYPE_INACTIVE][iter.key()] = pAction;
            }
            iter++;
            // remove from active (was copied above)?
            if(iterRemove != d->m_PoolActionsArray[ACTION_POOL_TYPE_LONG_OBSERVE].end())
                iter = d->m_PoolActionsArray[ACTION_POOL_TYPE_LONG_OBSERVE].erase(iterRemove);
            else
                iter++;
        }
        // notify our users
        if(bLongTermObservationFailed)
            emit LongTermObservationError();
    }
}
