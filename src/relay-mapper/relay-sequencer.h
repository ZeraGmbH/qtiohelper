#ifndef QRelaySequencer_H
#define QRelaySequencer_H

#include "relay-base.h"
#include <QVector>

enum enRelaisSequencerSwitchTypes
{
    /* SWITCH_TRANSPARENT example transition:
     * Initial    Intermediate    End
     * 0000 --------------------> 1010
     */
    SWITCH_TRANSPARENT = 0, // only for test

    /* SWITCH_OVERLAPPED_ON example transition:
     * Initial    Intermediate    End
     * 1001 --->  1101 ---------> 0101
     */
    SWITCH_OVERLAPPED_ON,

    /* SWITCH_OVERLAPPED_OFF example transition:
     * Initial    Intermediate    End
     * 1001 --->  0001 ---------> 0101
     */
    SWITCH_OVERLAPPED_OFF,

    /* SWITCH_PASS_ON example transition:
     * Initial    Intermediate    End
     * 1001 --->  1111 ---------> 0101
     */
    SWITCH_PASS_OFF,

    /* SWITCH_PASS_ON example transition:
     * Initial    Intermediate    End
     * 1001 --->  0000 ---------> 0101
     */
    SWITCH_PASS_ON,

    SWITCH_UNDEF
};


class QRelaySequencerPrivate;

class QTRELAYSSHARED_EXPORT QRelaySequencer : public QRelayBase
{
    Q_OBJECT
public:
    QRelaySequencer(QObject *parent = Q_NULLPTR);
    // A group of pins set in arrui16MemberRelayNums switches as set in relaisSequencerSwitchType
    void setup(quint16 ui16LogicalArrayInfoCount,
               enum enRelaisSequencerSwitchTypes relaisSequencerSwitchType,
               QVector<quint16> arrui16MemberLogRelayNums,
               RelayStartLowLayerSwitchFunction CallbackStartLowLayerSwitch);

    virtual void startSet(quint16 ui16BitNo,
                          bool bSet,
                          bool bForce = false);

public slots:
    virtual void onLowLayerIdle();

private slots:
    void onIdleTimer();

private:
    Q_DECLARE_PRIVATE(QRelaySequencer)
};

#endif // QRelaySequencer_H
