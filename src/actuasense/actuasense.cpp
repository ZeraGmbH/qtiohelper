#include "actuasense.h"
#include "actuasense_p.h"

// ********************************* QActuaSenseIOData *********************************
QActuaSenseIOData::QActuaSenseIOData(QObject* pParent) : QObject(pParent)
{
    m_iInBitNum = -1;
    m_iOutBitNum = -1;

    m_bDemoMode = false;
    m_iDemoBitNumIn = -1;
    m_pDemoBitArrIn = Q_NULLPTR;
    m_bDemoError = false;

    m_iTimeoutMs = 0;
    m_bInStateDesired = false;
    m_iMsSinceLastSet = 0;

    m_eActionState = ACTION_STATE_INACTIVE;
}

void QActuaSenseIOData::setDemoInfo(bool bDemoMode, QBitArray *pDemoBitArrIn, int iDemoBitNumIn)
{
    m_bDemoMode = bDemoMode;
    m_iDemoBitNumIn = iDemoBitNumIn;
    m_pDemoBitArrIn = pDemoBitArrIn;
}


// ********************************* QActuaSensePrivate *********************************
QActuaSensePrivate::QActuaSensePrivate(QActuaSense *pPublic) : q_ptr(pPublic)
{
    m_pInBitArr = Q_NULLPTR;
    m_pLowLayerStartFunc = Q_NULLPTR;
}

QActuaSensePrivate::~QActuaSensePrivate()
{
    for(QActuaSenseIODataIntHash::iterator iter=m_PoolIOData.begin(); iter!=m_PoolIOData.end(); iter++)
        delete iter.value();
    m_PoolIOData.clear();
}

void QActuaSensePrivate::openMultiAction()
{
    // if the last multi action failed we have entries left in active
    // move them to inactive
    for(QActuaSenseIODataIntHash::iterator iter = m_PoolIOData.begin();
        iter != m_PoolIOData.end();
        iter++)
    {
        QActuaSenseIOData *pActionData = iter.value();
        if(pActionData->m_eActionState == ACTION_STATE_ACTIVE)
            pActionData->m_eActionState = ACTION_STATE_INACTIVE;
    }
    m_bInAddingActions = true;
}

void QActuaSensePrivate::startOutSet(int iActionID, bool bStateOut)
{
    QActuaSenseIODataIntHash::iterator iter = m_PoolIOData.find(iActionID);
    if(iter != m_PoolIOData.end())
    {
        QActuaSenseIOData* pActionData = iter.value();
        if(pActionData->m_iOutBitNum>=0)
        {
            m_OutEnableBitArr.setBit(pActionData->m_iOutBitNum);
            m_OutSetBitArr.setBit(pActionData->m_iOutBitNum, bStateOut);
            // if we are not in a multi action we transfer now otherwise keep set data
            // and transfer in closeMultiAction
            if(!m_bInAddingActions && m_pLowLayerStartFunc)
            {
                // just fire and forget
                m_pLowLayerStartFunc(m_OutEnableBitArr, m_OutSetBitArr);
                // avoid setting this bit in next transaction
                m_OutEnableBitArr.clearBit(pActionData->m_iOutBitNum);
            }
        }
        else
            qWarning("startOutSet: iActionId %i no out information set!", iActionID);
    }
    else
        qWarning("startOutSet: iActionId %i not found!", iActionID);
}

void QActuaSensePrivate::startInObserve(int iActionID, bool bStateInDesired, int iTimeoutMs, QString strOK, QString strErr, QString strLongTermErr)
{
    QActuaSenseIODataIntHash::iterator iter = m_PoolIOData.find(iActionID);
    if(iter != m_PoolIOData.end())
    {
        QActuaSenseIOData *pActionData = iter.value();
        // We are inside a transaction -> active
        if(m_bInAddingActions)
            pActionData->m_eActionState = ACTION_STATE_ACTIVE;
        // outside transaction
        else
        {
            // move to long term if error message is set
            if(!strLongTermErr.isEmpty())
                pActionData->m_eActionState = ACTION_STATE_LONG_OBSERVE;
            // move to inactive
            else
                pActionData->m_eActionState = ACTION_STATE_INACTIVE;
        }
        if(pActionData)
        {
            pActionData->m_iTimeoutMs = iTimeoutMs;
            pActionData->m_iMsSinceLastSet = 0;
            pActionData->m_bInStateDesired = bStateInDesired;
            pActionData->m_strOK = strOK;
            pActionData->m_strErr = strErr;
            pActionData->m_strLongTermErr = strLongTermErr;
        }
    }
    else
        qWarning("startInObserve: iActionId %i not found!", iActionID);

}

void QActuaSensePrivate::closeMultiAction()
{
    m_bInAddingActions = false;
    if(m_pLowLayerStartFunc && m_OutEnableBitArr.count(true) != 0)
    {
        // start low layer transaction
        // note: we don't care if low layer call does block or not - multiActionFinished by
        // input conditions polled
        m_pLowLayerStartFunc(m_OutEnableBitArr, m_OutSetBitArr);
        // reset flags to avoid setting in next transaction
        m_OutEnableBitArr.fill(false);
    }

}

bool QActuaSensePrivate::readInputState(int iActionID)
{
    bool bInState = false;
    QActuaSenseIODataIntHash::iterator iter = m_PoolIOData.find(iActionID);
    if(iter != m_PoolIOData.end())
    {

        QActuaSenseIOData* pActionData = iter.value();
        if(pActionData->m_iInBitNum >= 0)
            bInState = readInputState(pActionData);
        else
            qWarning("No input set for action %i", iActionID);
    }
    else
        qWarning("readInputState: iActionId %i not found!", iActionID);
    return bInState;
}

bool QActuaSensePrivate::hasReachedDestinationState(QActuaSenseIOData *pActionData)
{
    // In case no input pin was set: use timeout
    if(pActionData->m_iInBitNum < 0)
        return hasTimedOut(pActionData);
    return pActionData->m_bInStateDesired == readInputState(pActionData);
}

bool QActuaSensePrivate::hasTimedOut(QActuaSenseIOData *pActionData)
{
    return pActionData->m_iMsSinceLastSet >= pActionData->m_iTimeoutMs;
}

bool QActuaSensePrivate::getLongTermStatus(QString &strErr, bool bNotifyOnce)
{
    bool bLongTermError = false;
    strErr.clear();
    for(QActuaSenseIODataIntHash::iterator iter = m_PoolIOData.begin();
        iter != m_PoolIOData.end();
        iter++)
    {
        QActuaSenseIOData* pActionData = iter.value();
        if(pActionData->m_eActionState == ACTION_STATE_LONG_OBSERVE_ERR)
        {
            bLongTermError = true;
            if(strErr.isEmpty())
                strErr = pActionData->m_strLongTermErr;
            else
                strErr += QLatin1String(" / ") + pActionData->m_strLongTermErr;
            if(bNotifyOnce)
                pActionData->m_eActionState = ACTION_STATE_INACTIVE;
        }
    }
    return bLongTermError;
}

void QActuaSensePrivate::onPollTimer()
{
    int iTimeSinceLastPoll = m_TimerElapsedLastPoll.restart();
    bool bActiveError = false;
    bool bLongTermError = false;
    bool bOneOrMoreFinished = false;
    // 1st loop over all action arrays
    for(QActuaSenseIODataIntHash::iterator iter = m_PoolIOData.begin();
        iter != m_PoolIOData.end();
        iter++)
    {
        QActuaSenseIOData* pActionData = iter.value();
        // active actions
        if(!m_bInAddingActions && pActionData->m_eActionState == ACTION_STATE_ACTIVE)
        {
            pActionData->m_iMsSinceLastSet += iTimeSinceLastPoll;
            // action finished?
            if(hasReachedDestinationState(pActionData))
            {
                bOneOrMoreFinished = true;
                // Drop OK string
                if(!pActionData->m_strOK.isEmpty())
                    qInfo(pActionData->m_strOK.toLatin1());
                // pass to long term observation if error notification is desired
                if(!pActionData->m_strLongTermErr.isEmpty())
                    pActionData->m_eActionState = ACTION_STATE_LONG_OBSERVE;
                // pass to inactive
                else
                    pActionData->m_eActionState = ACTION_STATE_INACTIVE;
            }
            // action timed out?
            else if(hasTimedOut(pActionData))
            {
                bOneOrMoreFinished = true;
                // treat as error only for error string set
                if(!pActionData->m_strErr.isEmpty())
                {
                    bActiveError = true;
                    qCritical(pActionData->m_strErr.toLatin1());
                }
                pActionData->m_eActionState = ACTION_STATE_INACTIVE;
            }
        }
        // long term observation
        if(pActionData->m_eActionState == ACTION_STATE_LONG_OBSERVE)
        {
            // Lost it's destination state?
            if(!hasReachedDestinationState(pActionData))
            {
                qCritical(pActionData->m_strLongTermErr.toLatin1());
                bLongTermError = true;
                // pass to long observe error
                pActionData->m_eActionState = ACTION_STATE_LONG_OBSERVE_ERR;
            }
        }
    }

    // another loop
    // 1. in case of active error: move all remaining active actions -> inactive to avoid multiple fire
    // 2. no more active left -> notify
    bool bActivePending = false;
    for(QActuaSenseIODataIntHash::iterator iter = m_PoolIOData.begin();
        iter != m_PoolIOData.end();
        iter++)
    {
        QActuaSenseIOData* pActionData = iter.value();
        if(pActionData->m_eActionState == ACTION_STATE_ACTIVE)
        {
            // 1.
            if(bActiveError)
                pActionData->m_eActionState = ACTION_STATE_INACTIVE;
            // 2.
            else
                bActivePending = true;
        }
    }

    Q_Q(QActuaSense);
    // active actions finished -> notification
    if(bOneOrMoreFinished && !bActivePending)
        emit q->multiActionFinished(bActiveError);

    // notify for long term errors
    if(bLongTermError)
        emit q->longTermObservationError();

}

bool QActuaSensePrivate::readInputState(QActuaSenseIOData *pActionData)
{
    bool bInState = false;
    if(!pActionData->m_bDemoMode)
        bInState = m_pInBitArr->testBit(pActionData->m_iInBitNum);
    // demo mode
    else
    {
        // full input demo
        if(pActionData->m_pDemoBitArrIn && pActionData->m_iDemoBitNumIn >=0)
            bInState = pActionData->m_pDemoBitArrIn->testBit(pActionData->m_iDemoBitNumIn);
        // demo error
        else if(pActionData->m_bDemoError)
            bInState = !pActionData->m_bInStateDesired;
        // standard demo
        else
        {
            // standard demo reaches desired state at half timeout
            if(pActionData->m_iMsSinceLastSet >= pActionData->m_iTimeoutMs/2 )
                bInState = pActionData->m_bInStateDesired;
            else
                bInState = !pActionData->m_bInStateDesired;
        }
    }
    return bInState;
}

void QActuaSensePrivate::removeFromLongObserv(int iActionID)
{
    QActuaSenseIODataIntHash::iterator iter;
    QActuaSenseIOData* pActionData;
    // one
    if(iActionID != -1)
    {
        iter = m_PoolIOData.find(iActionID);
        if(iter != m_PoolIOData.end())
        {
            pActionData = iter.value();
            if( pActionData->m_eActionState == ACTION_STATE_LONG_OBSERVE ||
                pActionData->m_eActionState == ACTION_STATE_LONG_OBSERVE_ERR )
                pActionData->m_eActionState = ACTION_STATE_INACTIVE;
        }
    }
    // all
    else
    {
        for(iter = m_PoolIOData.begin();
            iter != m_PoolIOData.end();
            iter++)
        {
            pActionData = iter.value();
            if( pActionData->m_eActionState == ACTION_STATE_LONG_OBSERVE ||
                pActionData->m_eActionState == ACTION_STATE_LONG_OBSERVE_ERR )
                pActionData->m_eActionState = ACTION_STATE_INACTIVE;
        }
    }

}

QActuaSenseIOData* QActuaSensePrivate::findOrCreateIOParam(int iActionID, QObject* pParent)
{
    QActuaSenseIODataIntHash::iterator iter = m_PoolIOData.find(iActionID);
    QActuaSenseIOData* pActionData;
    if(iter == m_PoolIOData.end())
    {
        // create a new parameter
        pActionData = new QActuaSenseIOData(pParent);
        m_PoolIOData[iActionID] = pActionData;
    }
    else
        pActionData = iter.value();
    return pActionData;
}

// ********************************* QActuaSense *********************************

QActuaSense::QActuaSense(QObject *parent) :
    QObject(parent),
    d_ptr(new QActuaSensePrivate(this))
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
    QActuaSenseIOData* pActionData = d->findOrCreateIOParam(iActionID, this);
    pActionData->m_iInBitNum = iInBitNum;
}

void QActuaSense::addAtomicOut(int iActionID, int iOutBitNum)
{
    Q_D(QActuaSense);
    QActuaSenseIOData* pActionData = d->findOrCreateIOParam(iActionID, this);
    pActionData->m_iOutBitNum = iOutBitNum;
    // bitmaps size alignment neccessary?
    if(iOutBitNum+1 > d->m_OutSetBitArr.count())
    {
        d->m_OutEnableBitArr.resize(iOutBitNum+1);
        d->m_OutSetBitArr.resize(iOutBitNum+1);
    }
}

void QActuaSense::setBitMaskSize(int iSize)
{
    Q_D(QActuaSense);
    d->m_OutEnableBitArr.resize(iSize);
    d->m_OutSetBitArr.resize(iSize);
}

void QActuaSense::setupDemo(bool bDemoMode, int iActionID, QBitArray *pDemoBitArrIn, int iDemoBitNumIn)
{
    Q_D(QActuaSense);
    QActuaSenseIODataIntHash::iterator iter;
    QActuaSenseIOData* pActionData;
    // one
    if(iActionID != -1)
    {
        iter = d->m_PoolIOData.find(iActionID);
        if(iter != d->m_PoolIOData.end())
        {
            pActionData = iter.value();
            pActionData->m_bDemoMode = bDemoMode;
            pActionData->m_pDemoBitArrIn = pDemoBitArrIn;
            pActionData->m_iDemoBitNumIn = iDemoBitNumIn;
        }
    }
    // all
    else
    {
        for(iter = d->m_PoolIOData.begin(); iter!=d->m_PoolIOData.end(); iter++)
        {
            pActionData = iter.value();
            pActionData->m_bDemoMode = bDemoMode;
            pActionData->m_pDemoBitArrIn = pDemoBitArrIn;
            pActionData->m_iDemoBitNumIn = iDemoBitNumIn;
        }
    }
}

void QActuaSense::setDemoError(bool bDemoError, int iActionID)
{
    Q_D(QActuaSense);
    QActuaSenseIODataIntHash::iterator iter;
    QActuaSenseIOData* pActionData;
    // one
    if(iActionID != -1)
    {
        iter = d->m_PoolIOData.find(iActionID);
        if(iter != d->m_PoolIOData.end())
        {
            pActionData = iter.value();
            pActionData->m_bDemoError = bDemoError;
        }
    }
    // all
    else
    {
        for(iter = d->m_PoolIOData.begin(); iter!=d->m_PoolIOData.end(); iter++)
        {
            pActionData = iter.value();
            pActionData->m_bDemoError = bDemoError;
        }
    }
}

void QActuaSense::startInternalInputPoll(int milliSeconds)
{
    Q_D(QActuaSense);
    d->m_IoPollTimer.setSingleShot(false);
    d->m_IoPollTimer.start(milliSeconds);
    connect(&d->m_IoPollTimer,&QTimer::timeout, this, &QActuaSense::onPollTimer);
}

void QActuaSense::stopInternalInputPoll()
{
    Q_D(QActuaSense);
    d->m_IoPollTimer.stop();
    disconnect(&d->m_IoPollTimer,&QTimer::timeout, this, &QActuaSense::onPollTimer);
}

void QActuaSense::openMultiAction()
{
    Q_D(QActuaSense);
    d->openMultiAction();
}

void QActuaSense::startOutSet(int iActionID, bool bStateOut)
{
    Q_D(QActuaSense);
    d->startOutSet(iActionID, bStateOut);
}

void QActuaSense::startInObserve(int iActionID,
                                 bool bStateInDesired,
                                 int iTimeoutMs,
                                 QString strOK, QString strErr, QString strLongTermErr)
{
    Q_D(QActuaSense);
    d->startInObserve(iActionID, bStateInDesired, iTimeoutMs, strOK, strErr, strLongTermErr);
}

void QActuaSense::closeMultiAction()
{
    Q_D(QActuaSense);
    d->closeMultiAction();
}

bool QActuaSense::readInputState(int iActionID)
{
    Q_D(QActuaSense);
    return d->readInputState(iActionID);
}

void QActuaSense::removeFromLongObserv(int iActionID)
{
    Q_D(QActuaSense);
    d->removeFromLongObserv(iActionID);
}

bool QActuaSense::getLongTermStatus(QString &strErr)
{
    Q_D(QActuaSense);
    return d->getLongTermStatus(strErr);
}

void QActuaSense::onPollTimer()
{
    Q_D(QActuaSense);
    d->onPollTimer();
}
