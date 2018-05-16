#ifndef QRelaySequencer_H
#define QRelaySequencer_H

#include "relay-base.h"
#include <QVector>

enum enRelaySequencerSwitchTypes
{
    /* SWITCH_TRANSPARENT example transition:
     * Initial    Intermediate    End
     * 0000 --------------------> 1010
     */
    SWITCH_TRANSPARENT = 0, // default if bit is not member of any group

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
    SWITCH_PASS_ON,

    /* SWITCH_PASS_OFF example transition:
     * Initial    Intermediate    End
     * 1001 --->  0000 ---------> 0101
     */
    SWITCH_PASS_OFF,

    SWITCH_UNDEF
};

struct TRelaySequencerGroup
{
    TRelaySequencerGroup()
    {
        relaySequencerSwitchType = SWITCH_TRANSPARENT;
    }
    TRelaySequencerGroup(enum enRelaySequencerSwitchTypes relaySequencerSwitchType_, QVector<quint16> arrui16MemberLogRelayNums_ )
    {
        relaySequencerSwitchType = relaySequencerSwitchType_;
        arrui16MemberLogRelayNums = arrui16MemberLogRelayNums_;
    }
    enum enRelaySequencerSwitchTypes relaySequencerSwitchType;
    QVector<quint16> arrui16MemberLogRelayNums;
};

class QRelaySequencerPrivate;

class QTRELAYSSHARED_EXPORT QRelaySequencer : public QRelayUpperBase
{
    Q_OBJECT
public:
    QRelaySequencer(QObject *parent = Q_NULLPTR);

    bool addGroup(const TRelaySequencerGroup &group);

protected:
    virtual void setupBaseBitmaps(quint16 ui16LogicalArrayInfoCount);
    virtual bool process();

private:
    Q_DECLARE_PRIVATE(QRelaySequencer)
};

#endif // QRelaySequencer_H
