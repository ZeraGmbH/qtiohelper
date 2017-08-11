#include "relay-sequencer.h"
#include "relay-sequencer_p.h"

// ************************** QRelaySequencerPrivate

QRelaySequencerPrivate::QRelaySequencerPrivate()
{
}

QRelaySequencerPrivate::~QRelaySequencerPrivate()
{
}

// ************************** QRelaySequencer

QRelaySequencer::QRelaySequencer(QObject *parent) :
    QRelayBase(parent, new QRelaySequencerPrivate())
{
    Q_D(QRelaySequencer);
    // setup out idle timer
    d->m_IdleTimer.setSingleShot(true);
    d->m_IdleTimer.setInterval(0);
    connect(&d->m_IdleTimer, &QTimer::timeout, this, &QRelaySequencer::onIdleTimer);
}

void QRelaySequencer::setup(quint16 ui16LogicalArrayInfoCount,
                            enum enRelaisSequencerSwitchTypes relaisSequencerSwitchType,
                            QVector<quint16> arrui16MemberLogRelayNums,
                            RelayStartLowLayerSwitchFunction CallbackStartLowLayerSwitch)
{
    Q_D(QRelaySequencer);

    QRelayBase::setup(ui16LogicalArrayInfoCount, CallbackStartLowLayerSwitch);
    d->relaisSequencerSwitchType = relaisSequencerSwitchType;
    d->arrui16MemberLogRelayNums = arrui16MemberLogRelayNums;
}

void QRelaySequencer::startSet(quint16 ui16BitNo,
                               bool bSet,
                               bool bForce)
{
    QRelayBase::startSet(ui16BitNo, bSet, bForce);

    Q_D(QRelaySequencer);
    d->m_IdleTimer.start();
}

void QRelaySequencer::onIdleTimer()
{
    Q_D(QRelaySequencer);
    // in case low layer is still busy - wait for next idle
    if(d->CallbackQueryLowLayerBusy && d->CallbackQueryLowLayerBusy())
        return;

}

void QRelaySequencer::onLowLayerIdle()
{
    Q_D(QRelayBase);
    if(!isBusy())
    {
        // All transactions done -> give notification
        idleCleanup();
        emit idle();
    }
    else
    {

    }
}
