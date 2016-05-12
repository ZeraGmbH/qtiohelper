#include "actuasense.h"
#include "actuasense_p.h"

// ********************************* QActuaSenseIOParams *********************************
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


// ********************************* QActuaSenseAction *********************************
QActuaSenseAction::QActuaSenseAction(QActuaSenseIOParams* pAtomicIO) : QObject(pAtomicIO)
{
    m_pIOParams = pAtomicIO;
    m_iTimeoutMs = 0;
    m_bInStateDesired = false;
    m_iMsSinceLastSet = 0;
}

// ********************************* QActuaSenseActionPointerArray *********************************
QActuaSenseActionPointerArray::QActuaSenseActionPointerArray(QActuaSenseAction *pAction)
{
    for(int iPointer=0; iPointer<ACTION_POOL_TYPE_COUNT; iPointer++)
        m_arrpAction[iPointer] = NULL;
    m_arrpAction[ACTION_POOL_TYPE_INACTIVE] = pAction;
}

QActuaSenseActionPointerArray::~QActuaSenseActionPointerArray()
{
    for(int iPointer=0; iPointer<ACTION_POOL_TYPE_COUNT; iPointer++)
        if(m_arrpAction[iPointer])
            delete m_arrpAction[iPointer];
}

QActuaSenseAction* QActuaSenseActionPointerArray::moveActionTo(enum enActionPoolType eNewType)
{
    QActuaSenseAction* pAction = NULL;
    for(int iPointer=0; iPointer<ACTION_POOL_TYPE_COUNT; iPointer++)
    {
        if(m_arrpAction[iPointer])
        {
            pAction = m_arrpAction[iPointer];
            m_arrpAction[iPointer] = NULL;
            break;
        }
    }
    if(pAction)
        m_arrpAction[eNewType] = pAction;
    else
        qFatal("QActuaSenseActionPointerArray::moveActionTo: There were no action pointers found!");
    return pAction;
}

QActuaSenseAction* QActuaSenseActionPointerArray::find()
{
    QActuaSenseAction* pAction = NULL;
    for(int iPointer=0; iPointer<ACTION_POOL_TYPE_COUNT; iPointer++)
    {
        if(m_arrpAction[iPointer])
        {
            pAction = m_arrpAction[iPointer];
            break;
        }
    }
    if(pAction == NULL)
        qFatal("QActuaSenseActionPointerArray::find: There were no action pointers found!");
    return pAction;
}


// ********************************* QActuaSensePrivate *********************************
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

bool QActuaSensePrivate::hasReachedDestinationState(QActuaSenseAction *pAction)
{
    // In case no input pin was set: use timeout
    if(pAction->m_pIOParams->m_iInBitNum < 0)
        return hasTimedOut(pAction);
    return pAction->m_bInStateDesired == readInputState(pAction);
}

bool QActuaSensePrivate::hasTimedOut(QActuaSenseAction *pAction)
{
    return pAction->m_iMsSinceLastSet >= pAction->m_iTimeoutMs;
}

bool QActuaSensePrivate::readInputState(QActuaSenseAction *pAction)
{
    bool bInState = false;
    if(!pAction->m_pIOParams->m_bDemoMode)
        bInState = m_pInBitArr->at(pAction->m_pIOParams->m_iInBitNum);
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



// ********************************* QActuaSense *********************************

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
        // create a new parameter
        pIOParams = new QActuaSenseIOParams(this);
    else
        pIOParams = iter.value();
    pIOParams->setIn(iInBitNum);

    // Do we need to create an action entry?
    QActuaSenseActionPointerArrayIntHash::iterator iterActionPointerArray = d->m_PoolActionsArray.find(iActionID);
    if(iterActionPointerArray == d->m_PoolActionsArray.end())
        d->m_PoolActionsArray[iActionID] =
                new QActuaSenseActionPointerArray(new QActuaSenseAction(pIOParams));
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
    for(QActuaSenseActionPointerArrayIntHash::iterator iter = d->m_PoolActionsArray.begin();
        iter != d->m_PoolActionsArray.end();
        iter++)
    {
        QActuaSenseActionPointerArray *pActionPointerArray = iter.value();
        if(pActionPointerArray->m_arrpAction[ACTION_POOL_TYPE_ACTIVE])
        {
            pActionPointerArray->m_arrpAction[ACTION_POOL_TYPE_INACTIVE] =
                    pActionPointerArray->m_arrpAction[ACTION_POOL_TYPE_ACTIVE];
            pActionPointerArray->m_arrpAction[ACTION_POOL_TYPE_ACTIVE] = NULL;
        }
    }
    d->m_bInAddingActions = true;
}

void QActuaSense::startOutSet(int iActionID, bool bStateOut)
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
            qWarning("startOutSet: iActionId %i no out information set!", iActionID);
    }
    else
        qWarning("startOutSet: iActionId %i not found!", iActionID);
}

void QActuaSense::startInObserve(int iActionID,
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
            pAction = d->m_PoolActionsArray[iActionID]->moveActionTo(ACTION_POOL_TYPE_ACTIVE);
        // outside transaction
        else
        {
            // move to long term if error message is set
            if(!strLongTermErr.isEmpty())
                pAction = d->m_PoolActionsArray[iActionID]->moveActionTo(ACTION_POOL_TYPE_LONG_OBSERVE);
            // move to inactive
            else
                pAction = d->m_PoolActionsArray[iActionID]->moveActionTo(ACTION_POOL_TYPE_INACTIVE);
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
        qWarning("startInObserve: iActionId %i not found!", iActionID);
}

void QActuaSense::closeMultiAction()
{
    Q_D(QActuaSense);
    d->m_bInAddingActions = false;
    if(d->m_pLowLayerStartFunc && d->m_OutEnableBitArr.count(true) != 0)
    {
        // start low layer transaction
        // note: we don't care if low layer call does block or not - multiActionFinished by
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
            QActuaSenseActionPointerArrayIntHash::iterator iterActionPointerArrays = d->m_PoolActionsArray.find(iActionID);
            if(iterActionPointerArrays != d->m_PoolActionsArray.end())
                pAction = iterActionPointerArrays.value()->find();
            bInState = d->readInputState(pAction);
        }
        else
            qWarning("No input set for action %i", iActionID);
    }
    else
        qWarning("readInputState: iActionId %i not found!", iActionID);
    return bInState;
}

void QActuaSense::removeFromLongObserv(int iActionID)
{
    Q_D(QActuaSense);
    QActuaSenseActionPointerArrayIntHash::iterator iterActionPointerArray;
    QActuaSenseActionPointerArray* pActionPointerArray;
    // one
    if(iActionID != -1)
    {
        iterActionPointerArray = d->m_PoolActionsArray.find(iActionID);
        if(iterActionPointerArray != d->m_PoolActionsArray.end())
        {
            pActionPointerArray = iterActionPointerArray.value();
            if(pActionPointerArray->m_arrpAction[ACTION_POOL_TYPE_LONG_OBSERVE])
            {
                pActionPointerArray->m_arrpAction[ACTION_POOL_TYPE_INACTIVE] =
                        pActionPointerArray->m_arrpAction[ACTION_POOL_TYPE_LONG_OBSERVE];
                pActionPointerArray->m_arrpAction[ACTION_POOL_TYPE_LONG_OBSERVE] = NULL;
            }
        }
    }
    // all
    else
    {
        for(iterActionPointerArray = d->m_PoolActionsArray.begin();
            iterActionPointerArray != d->m_PoolActionsArray.end();
            iterActionPointerArray++)
        {
            pActionPointerArray = iterActionPointerArray.value();
            if(pActionPointerArray->m_arrpAction[ACTION_POOL_TYPE_LONG_OBSERVE])
            {
                pActionPointerArray->m_arrpAction[ACTION_POOL_TYPE_INACTIVE] =
                        pActionPointerArray->m_arrpAction[ACTION_POOL_TYPE_LONG_OBSERVE];
                pActionPointerArray->m_arrpAction[ACTION_POOL_TYPE_LONG_OBSERVE] = NULL;
            }
        }
    }

}

void QActuaSense::onPollTimer()
{
    Q_D(QActuaSense);
    int iTimeSinceLastPoll = d->m_TimerElapsedLastPoll.restart();
    bool bActiveError = false;
    bool bLongTermError = false;
    bool bOneOrMoreFinished = false;
    // 1st loop over all action arrays
    for(QActuaSenseActionPointerArrayIntHash::iterator iter = d->m_PoolActionsArray.begin();
        iter != d->m_PoolActionsArray.end();
        iter++)
    {
        QActuaSenseAction* pAction = NULL;
        QActuaSenseActionPointerArray* pActionPointerArray = iter.value();
        // active actions
        if(!d->m_bInAddingActions && pActionPointerArray->m_arrpAction[ACTION_POOL_TYPE_ACTIVE])
        {
            pAction = pActionPointerArray->m_arrpAction[ACTION_POOL_TYPE_ACTIVE];
            pAction->m_iMsSinceLastSet += iTimeSinceLastPoll;
            // action finished?
            if(d->hasReachedDestinationState(pAction))
            {
                bOneOrMoreFinished = true;
                // Drop OK string
                if(!pAction->m_strOK.isEmpty())
                    qInfo(pAction->m_strOK.toLatin1());
                // pass to long term observation if error notification is desired
                if(!pAction->m_strLongTermErr.isEmpty())
                    pActionPointerArray->moveActionTo(ACTION_POOL_TYPE_LONG_OBSERVE);
                // pass to inactive
                else
                    pActionPointerArray->moveActionTo(ACTION_POOL_TYPE_INACTIVE);
            }
            // action timed out?
            else if(d->hasTimedOut(pAction))
            {
                // treat as error only for error string set
                if(!pAction->m_strErr.isEmpty())
                {
                    bActiveError = true;
                    qCritical(pAction->m_strErr.toLatin1());
                }
                // pass to inactive
                pActionPointerArray->moveActionTo(ACTION_POOL_TYPE_INACTIVE);
            }
        }
        // long term observation
        if(pActionPointerArray->m_arrpAction[ACTION_POOL_TYPE_LONG_OBSERVE])
        {
            pAction = pActionPointerArray->m_arrpAction[ACTION_POOL_TYPE_LONG_OBSERVE];
            // Lost it's destination state?
            if(!d->hasReachedDestinationState(pAction))
            {
                if(!pAction->m_strLongTermErr.isEmpty())
                    qCritical(pAction->m_strLongTermErr.toLatin1());
                bLongTermError = true;
                // pass to inactive
                pActionPointerArray->moveActionTo(ACTION_POOL_TYPE_INACTIVE);
            }
        }
    }

    // another loop
    // 1. in case of active error: move all remaining active actions -> inactive to avoid multiple fire
    // 2. no more active left -> notify
    bool bActivePending = false;
    for(QActuaSenseActionPointerArrayIntHash::iterator iter = d->m_PoolActionsArray.begin();
        iter != d->m_PoolActionsArray.end();
        iter++)
    {
        QActuaSenseActionPointerArray* pActionPointerArray = iter.value();
        // 1.
        if(bActiveError && pActionPointerArray->m_arrpAction[ACTION_POOL_TYPE_ACTIVE])
            pActionPointerArray->moveActionTo(ACTION_POOL_TYPE_INACTIVE);
        // 2.
        if(pActionPointerArray->m_arrpAction[ACTION_POOL_TYPE_ACTIVE])
            bActivePending = true;
    }

    // active actions finished -> notification
    if(bOneOrMoreFinished && !bActivePending)
        emit multiActionFinished(bActiveError);

    // notify for log term errors
    if(bLongTermError)
        emit longTermObservationError();
}
