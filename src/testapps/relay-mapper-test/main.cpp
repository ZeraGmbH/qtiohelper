#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTimer>
#include <QtDebug>
#include <QElapsedTimer>
#include <math.h>
#include "../../relay-mapper/relay-mapper.h"
#include "../../relay-mapper/relay-sequencer.h"

// slice timer period should be
// * less than 1/5 of shortest relay
// * a common divider for all delays
const int sliceTimerPeriod = 10;

//////////////////////////////////////////////////////////////////
/// Relay setup for relay mapper only tests
enum enLogicalRelayDefinitionForMapper
{
    RELAY_BISTABLE_100_1 = 0,
    RELAY_BISTABLE_100_2,
    RELAY_BISTABLE_50_1,
    RELAY_BISTABLE_50_2,

    RELAY_MONOSTABLE_300_1,
    RELAY_MONOSTABLE_300_2,
    RELAY_MONOSTABLE_600_1,
    RELAY_MONOSTABLE_600_2,

    LOGICAL_RELAY_COUNT
};

enum enPhysPinsForMapper
{
    PIN_BISTABLE_100_1_ON = 0,
    PIN_BISTABLE_100_1_OFF,
    PIN_BISTABLE_100_2_ON,
    PIN_BISTABLE_100_2_OFF,
    PIN_BISTABLE_50_1_ON,
    PIN_BISTABLE_50_1_OFF,
    PIN_BISTABLE_50_2_ON,
    PIN_BISTABLE_50_2_OFF,

    PIN_MONOSTABLE_300_1,
    PIN_MONOSTABLE_300_2,
    PIN_MONOSTABLE_600_1,
    PIN_MONOSTABLE_600_2,

    PHYSICAL_PIN_COUNT
};

struct TLogicalRelayEntry arrRelayMapperSetup[LOGICAL_RELAY_COUNT];

static void initRelayMapperSetup()
{
    TLogicalRelayEntry* pEntry;

    pEntry = arrRelayMapperSetup + RELAY_BISTABLE_100_1;
    pEntry->ui16OnPosition  = PIN_BISTABLE_100_1_ON;
    pEntry->ui16OffPosition = PIN_BISTABLE_100_1_OFF;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(true /*bistable    */, false/*on not inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 100/sliceTimerPeriod; // 100ms

    pEntry = arrRelayMapperSetup + RELAY_BISTABLE_100_2;
    pEntry->ui16OnPosition  = PIN_BISTABLE_100_2_ON;
    pEntry->ui16OffPosition = PIN_BISTABLE_100_2_OFF;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(true /*bistable    */, true /*on     inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 100/sliceTimerPeriod; // 100ms

    pEntry = arrRelayMapperSetup + RELAY_BISTABLE_50_1;
    pEntry->ui16OnPosition  = PIN_BISTABLE_50_1_ON;
    pEntry->ui16OffPosition = PIN_BISTABLE_50_1_OFF;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(true /*bistable   */, false/*on not inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 50/sliceTimerPeriod; // 50ms

    pEntry = arrRelayMapperSetup + RELAY_BISTABLE_50_2;
    pEntry->ui16OnPosition  = PIN_BISTABLE_50_2_ON;
    pEntry->ui16OffPosition = PIN_BISTABLE_50_2_OFF;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(true /*bistable   */, false/*on not inverted*/, true /* off      inverted*/),
    pEntry->ui8OnTime = 50/sliceTimerPeriod; // 50ms

    pEntry = arrRelayMapperSetup + RELAY_MONOSTABLE_300_1;
    pEntry->ui16OnPosition  = PIN_MONOSTABLE_300_1;
    pEntry->ui16OffPosition = PIN_MONOSTABLE_300_1;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable*/, false/*on not inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 300/sliceTimerPeriod; // 300ms

    pEntry = arrRelayMapperSetup + RELAY_MONOSTABLE_300_2;
    pEntry->ui16OnPosition  = PIN_MONOSTABLE_300_2;
    pEntry->ui16OffPosition = PIN_MONOSTABLE_300_2;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable*/, true /*on     inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 300/sliceTimerPeriod; // 300ms

    pEntry = arrRelayMapperSetup + RELAY_MONOSTABLE_600_1;
    pEntry->ui16OnPosition  = PIN_MONOSTABLE_600_1;
    pEntry->ui16OffPosition = PIN_MONOSTABLE_600_1;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable*/, false/*on not inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 600/sliceTimerPeriod; // 600ms

    pEntry = arrRelayMapperSetup + RELAY_MONOSTABLE_600_2;
    pEntry->ui16OnPosition  = PIN_MONOSTABLE_600_2;
    pEntry->ui16OffPosition = PIN_MONOSTABLE_600_2;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable*/, false/*on not inverted*/, true /* off inverted -> should not cause any effect*/),
    pEntry->ui8OnTime = 600/sliceTimerPeriod; // 600ms
}

//////////////////////////////////////////////////////////////////
/// Relay setup for relay mapper + sequenceronly tests
enum enLogicalRelayDefinitionForSequencer
{
    RELAY_TRANSPARENT_1 = 0,
    RELAY_TRANSPARENT_2,
    RELAY_TRANSPARENT_3,
    RELAY_TRANSPARENT_4,

    RELAY_OVERLAPPED_ON_1,
    RELAY_OVERLAPPED_ON_2,
    RELAY_OVERLAPPED_ON_3,
    RELAY_OVERLAPPED_ON_4,

    RELAY_OVERLAPPED_OFF_1,
    RELAY_OVERLAPPED_OFF_2,
    RELAY_OVERLAPPED_OFF_3,
    RELAY_OVERLAPPED_OFF_4,

    RELAY_PASS_ON_1,
    RELAY_PASS_ON_2,
    RELAY_PASS_ON_3,
    RELAY_PASS_ON_4,

    RELAY_PASS_OFF_1,
    RELAY_PASS_OFF_2,
    RELAY_PASS_OFF_3,
    RELAY_PASS_OFF_4,

    LOGICAL_RELAY_COUNT_SEQUENCER
};

enum enPhysPinsForSequencer
{
    PIN_TRANSPARENT_1 = 0,
    PIN_TRANSPARENT_2,
    PIN_TRANSPARENT_3,
    PIN_TRANSPARENT_4,

    PIN_OVERLAPPED_ON_1,
    PIN_OVERLAPPED_ON_2,
    PIN_OVERLAPPED_ON_3,
    PIN_OVERLAPPED_ON_4,

    PIN_OVERLAPPED_OFF_1,
    PIN_OVERLAPPED_OFF_2,
    PIN_OVERLAPPED_OFF_3,
    PIN_OVERLAPPED_OFF_4,

    PIN_PASS_ON_1,
    PIN_PASS_ON_2,
    PIN_PASS_ON_3,
    PIN_PASS_ON_4,

    PIN_PASS_OFF_1,
    PIN_PASS_OFF_2,
    PIN_PASS_OFF_3,
    PIN_PASS_OFF_4,

    PHYSICAL_PIN_COUNT_SEQUENCER
};

struct TLogicalRelayEntry arrRelayMapperSetupSequencer[LOGICAL_RELAY_COUNT_SEQUENCER];

static void initRelayMapperSetupSequencer()
{
    TLogicalRelayEntry* pEntry;

    pEntry = arrRelayMapperSetupSequencer + RELAY_TRANSPARENT_1;
    pEntry->ui16OnPosition  = PIN_TRANSPARENT_1;
    pEntry->ui16OffPosition = PIN_TRANSPARENT_1;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable    */, false/*on not inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 100/sliceTimerPeriod; // 100ms

    pEntry = arrRelayMapperSetupSequencer + RELAY_TRANSPARENT_2;
    pEntry->ui16OnPosition  = PIN_TRANSPARENT_2;
    pEntry->ui16OffPosition = PIN_TRANSPARENT_2;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable    */, false/*on not inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 100/sliceTimerPeriod; // 100ms

    pEntry = arrRelayMapperSetupSequencer + RELAY_TRANSPARENT_3;
    pEntry->ui16OnPosition  = PIN_TRANSPARENT_3;
    pEntry->ui16OffPosition = PIN_TRANSPARENT_3;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable    */, false/*on not inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 100/sliceTimerPeriod; // 100ms

    pEntry = arrRelayMapperSetupSequencer + RELAY_TRANSPARENT_4;
    pEntry->ui16OnPosition  = PIN_TRANSPARENT_4;
    pEntry->ui16OffPosition = PIN_TRANSPARENT_4;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable    */, false/*on not inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 100/sliceTimerPeriod; // 100ms

    pEntry = arrRelayMapperSetupSequencer + RELAY_OVERLAPPED_ON_1;
    pEntry->ui16OnPosition  = PIN_OVERLAPPED_ON_1;
    pEntry->ui16OffPosition = PIN_OVERLAPPED_ON_1;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable    */, false/*on not inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 100/sliceTimerPeriod; // 100ms

    pEntry = arrRelayMapperSetupSequencer + RELAY_OVERLAPPED_ON_2;
    pEntry->ui16OnPosition  = PIN_OVERLAPPED_ON_2;
    pEntry->ui16OffPosition = PIN_OVERLAPPED_ON_2;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable    */, false/*on not inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 100/sliceTimerPeriod; // 100ms

    pEntry = arrRelayMapperSetupSequencer + RELAY_OVERLAPPED_ON_3;
    pEntry->ui16OnPosition  = PIN_OVERLAPPED_ON_3;
    pEntry->ui16OffPosition = PIN_OVERLAPPED_ON_3;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable    */, false/*on not inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 100/sliceTimerPeriod; // 100ms

    pEntry = arrRelayMapperSetupSequencer + RELAY_OVERLAPPED_ON_4;
    pEntry->ui16OnPosition  = PIN_OVERLAPPED_ON_4;
    pEntry->ui16OffPosition = PIN_OVERLAPPED_ON_4;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable    */, false/*on not inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 100/sliceTimerPeriod; // 100ms

    pEntry = arrRelayMapperSetupSequencer + RELAY_OVERLAPPED_OFF_1;
    pEntry->ui16OnPosition  = PIN_OVERLAPPED_OFF_1;
    pEntry->ui16OffPosition = PIN_OVERLAPPED_OFF_1;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable    */, false/*on not inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 100/sliceTimerPeriod; // 100ms

    pEntry = arrRelayMapperSetupSequencer + RELAY_OVERLAPPED_OFF_2;
    pEntry->ui16OnPosition  = PIN_OVERLAPPED_OFF_2;
    pEntry->ui16OffPosition = PIN_OVERLAPPED_OFF_2;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable    */, false/*on not inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 100/sliceTimerPeriod; // 100ms

    pEntry = arrRelayMapperSetupSequencer + RELAY_OVERLAPPED_OFF_3;
    pEntry->ui16OnPosition  = PIN_OVERLAPPED_OFF_3;
    pEntry->ui16OffPosition = PIN_OVERLAPPED_OFF_3;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable    */, false/*on not inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 100/sliceTimerPeriod; // 100ms

    pEntry = arrRelayMapperSetupSequencer + RELAY_OVERLAPPED_OFF_4;
    pEntry->ui16OnPosition  = PIN_OVERLAPPED_OFF_4;
    pEntry->ui16OffPosition = PIN_OVERLAPPED_OFF_4;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable    */, false/*on not inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 100/sliceTimerPeriod; // 100ms

    pEntry = arrRelayMapperSetupSequencer + RELAY_PASS_ON_1;
    pEntry->ui16OnPosition  = PIN_PASS_ON_1;
    pEntry->ui16OffPosition = PIN_PASS_ON_1;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable    */, false/*on not inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 100/sliceTimerPeriod; // 100ms

    pEntry = arrRelayMapperSetupSequencer + RELAY_PASS_ON_2;
    pEntry->ui16OnPosition  = PIN_PASS_ON_2;
    pEntry->ui16OffPosition = PIN_PASS_ON_2;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable    */, false/*on not inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 100/sliceTimerPeriod; // 100ms

    pEntry = arrRelayMapperSetupSequencer + RELAY_PASS_ON_3;
    pEntry->ui16OnPosition  = PIN_PASS_ON_3;
    pEntry->ui16OffPosition = PIN_PASS_ON_3;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable    */, false/*on not inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 100/sliceTimerPeriod; // 100ms

    pEntry = arrRelayMapperSetupSequencer + RELAY_PASS_ON_4;
    pEntry->ui16OnPosition  = PIN_PASS_ON_4;
    pEntry->ui16OffPosition = PIN_PASS_ON_4;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable    */, false/*on not inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 100/sliceTimerPeriod; // 100ms

    pEntry = arrRelayMapperSetupSequencer + RELAY_PASS_OFF_1;
    pEntry->ui16OnPosition  = PIN_PASS_OFF_1;
    pEntry->ui16OffPosition = PIN_PASS_OFF_1;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable    */, false/*on not inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 100/sliceTimerPeriod; // 100ms

    pEntry = arrRelayMapperSetupSequencer + RELAY_PASS_OFF_2;
    pEntry->ui16OnPosition  = PIN_PASS_OFF_2;
    pEntry->ui16OffPosition = PIN_PASS_OFF_2;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable    */, false/*on not inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 100/sliceTimerPeriod; // 100ms

    pEntry = arrRelayMapperSetupSequencer + RELAY_PASS_OFF_3;
    pEntry->ui16OnPosition  = PIN_PASS_OFF_3;
    pEntry->ui16OffPosition = PIN_PASS_OFF_3;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable    */, false/*on not inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 100/sliceTimerPeriod; // 100ms

    pEntry = arrRelayMapperSetupSequencer + RELAY_PASS_OFF_4;
    pEntry->ui16OnPosition  = PIN_PASS_OFF_4;
    pEntry->ui16OffPosition = PIN_PASS_OFF_4;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable    */, false/*on not inverted*/, false /* off not inverted*/),
    pEntry->ui8OnTime = 100/sliceTimerPeriod; // 100ms
}

struct TExpectedLowLayerData
{
    TExpectedLowLayerData(QBitArray &EnableMask_, QBitArray &SetMask_,  int delaySinceLast_)
    {
        EnableMask = EnableMask_;
        SetMask = SetMask_;
        delaySinceLast = delaySinceLast_;
    }
    QBitArray EnableMask;
    QBitArray SetMask;
    int delaySinceLast;
};

struct TTestCase
{
    QString Description;
    QBitArray EnableMask;
    QBitArray SetMask;
    QBitArray expectedLogicalCurrentMask;
    qint64 expectedFinalDelay;
    bool bForce;
    bool bSetMasksBitByBit;
    int lowLayerUnblockedDelayMs; // =0 -> blocked otherwise unblocked with delay set here
    QList<TExpectedLowLayerData> expectedData;
};

static void appendTestCasesMapper(QList<TTestCase> &testCases)
{
    QList<TExpectedLowLayerData> expectedData;
    TTestCase testCase;
    QBitArray resultEnableMask;
    QBitArray resultSetMask;

    /* define test case */
    testCase.Description = "Forced bistable 100ms->0";
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.expectedLogicalCurrentMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.bForce = true;
    testCase.bSetMasksBitByBit = false;
    testCase.lowLayerUnblockedDelayMs = 0; // blocked
    testCase.expectedFinalDelay = 0; // bistable only
    testCase.EnableMask.setBit(RELAY_BISTABLE_100_1, true);
    testCase.SetMask.setBit(RELAY_BISTABLE_100_1, false);    // force RELAY_BISTABLE_50_1 -> 0
    // init compare masks
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT);
    expectedData.clear();
    // low layer callback 1
    resultEnableMask.setBit(PIN_BISTABLE_100_1_OFF, true);
    resultSetMask.setBit(PIN_BISTABLE_100_1_OFF, true);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 0));
    // low layer callback 2
    resultSetMask.setBit(PIN_BISTABLE_100_1_OFF, false);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 100));
    // add testcase
    testCase.expectedData = expectedData;
    testCases.append(testCase);

    /* define test case */
    testCase.Description = "Forced bistable 100ms->0 (2nd time: check force)";
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.bForce = true;
    testCase.bSetMasksBitByBit = false;
    testCase.lowLayerUnblockedDelayMs = 0; // blocked
    testCase.EnableMask.setBit(RELAY_BISTABLE_100_1, true);
    testCase.SetMask.setBit(RELAY_BISTABLE_100_1, false);    // force RELAY_BISTABLE_50_1 -> 0
    // init compare masks
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT);
    expectedData.clear();
    // low layer callback 1
    resultEnableMask.setBit(PIN_BISTABLE_100_1_OFF, true);
    resultSetMask.setBit(PIN_BISTABLE_100_1_OFF, true);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 0));
    // low layer callback 2
    resultSetMask.setBit(PIN_BISTABLE_100_1_OFF, false);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 100));
    // add testcase
    testCase.expectedData = expectedData;
    testCases.append(testCase);

    /* define test case */
    testCase.Description = "Forced bistable 50ms->0 + ignore 100ms->1";
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.bForce = true;
    testCase.bSetMasksBitByBit = false;
    testCase.lowLayerUnblockedDelayMs = 0; // blocked
    testCase.expectedFinalDelay = 0; // bistable only
    testCase.EnableMask.setBit(RELAY_BISTABLE_50_1, true);
    testCase.SetMask.setBit(RELAY_BISTABLE_50_1, false); // force RELAY_BISTABLE_50_1 -> 0
    testCase.EnableMask.setBit(RELAY_BISTABLE_100_1, false);
    testCase.SetMask.setBit(RELAY_BISTABLE_100_1, true);  // ignore RELAY_BISTABLE_100_1 -> 1 (ignored)
    expectedData.clear();
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT);
    // low layer callback 1
    resultEnableMask.setBit(PIN_BISTABLE_50_1_OFF, true);
    resultSetMask.setBit(PIN_BISTABLE_50_1_OFF, true);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 0));
    // low layer callback 2
    resultSetMask.setBit(PIN_BISTABLE_50_1_OFF, false);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 50));
    // add testcase
    testCase.expectedData = expectedData;
    testCases.append(testCase);

    /* define test case */
    testCase.Description = "Bistable 50ms->1 / 100ms->1";
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.bForce = false;
    testCase.bSetMasksBitByBit = false;
    testCase.lowLayerUnblockedDelayMs = 0; // blocked
    testCase.expectedFinalDelay = 0; // bistable only
    testCase.EnableMask.setBit(RELAY_BISTABLE_100_1, true);
    testCase.SetMask.setBit(RELAY_BISTABLE_100_1, true);     // switch RELAY_BISTABLE_100_1 -> 1
    testCase.EnableMask.setBit(RELAY_BISTABLE_50_1, true);
    testCase.SetMask.setBit(RELAY_BISTABLE_50_1, true);     // switch RELAY_BISTABLE_50_1 -> 1
    testCase.expectedLogicalCurrentMask.setBit(RELAY_BISTABLE_100_1, true);
    testCase.expectedLogicalCurrentMask.setBit(RELAY_BISTABLE_50_1, true);
    // init compare masks
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT);
    expectedData.clear();
    // low layer callback 1
    resultEnableMask.setBit(PIN_BISTABLE_100_1_ON, true);
    resultSetMask.setBit(PIN_BISTABLE_100_1_ON, true);
    resultEnableMask.setBit(PIN_BISTABLE_50_1_ON, true);
    resultSetMask.setBit(PIN_BISTABLE_50_1_ON, true);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 0));
    // low layer callback 2
    resultEnableMask.setBit(PIN_BISTABLE_100_1_ON, false);
    resultSetMask.setBit(PIN_BISTABLE_100_1_ON, false);
    resultSetMask.setBit(PIN_BISTABLE_50_1_ON, false);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 50));
    // low layer callback 3
    resultEnableMask.setBit(PIN_BISTABLE_100_1_ON, true);
    resultSetMask.setBit(PIN_BISTABLE_100_1_ON, false);
    resultEnableMask.setBit(PIN_BISTABLE_50_1_ON, false);
    resultSetMask.setBit(PIN_BISTABLE_50_1_ON, false);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 100-50));
    // add testcase
    testCase.expectedData = expectedData;
    testCases.append(testCase);

    /* define test case */
    testCase.Description = "Bistable 50ms->1 (no change) / 100ms-> 0";
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.bForce = false;
    testCase.bSetMasksBitByBit = false;
    testCase.lowLayerUnblockedDelayMs = 0; // blocked
    testCase.expectedFinalDelay = 0; // bistable only
    testCase.EnableMask.setBit(RELAY_BISTABLE_100_1, true);
    testCase.SetMask.setBit(RELAY_BISTABLE_100_1, false);     // switch RELAY_BISTABLE_100_1 -> 0
    testCase.EnableMask.setBit(RELAY_BISTABLE_50_1, true);
    testCase.SetMask.setBit(RELAY_BISTABLE_50_1, true);     // switch RELAY_BISTABLE_50_1 -> 1 (no change)
    testCase.expectedLogicalCurrentMask.setBit(RELAY_BISTABLE_100_1, false);
    // init compare masks
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT);
    expectedData.clear();
    // low layer callback 1
    resultEnableMask.setBit(PIN_BISTABLE_100_1_OFF, true);
    resultSetMask.setBit(PIN_BISTABLE_100_1_OFF, true);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 0));
    // low layer callback 2
    resultSetMask.setBit(PIN_BISTABLE_100_1_OFF, false);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 100));
    // add testcase
    testCase.expectedData = expectedData;
    testCases.append(testCase);

    /* define test case */
    testCase.Description = "Force Monostable_300+600->0";
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.bForce = true;
    testCase.bSetMasksBitByBit = false;
    testCase.lowLayerUnblockedDelayMs = 0; // blocked
    testCase.expectedFinalDelay = 600; // monostable max(300,600)
    testCase.EnableMask.setBit(RELAY_MONOSTABLE_300_1, true);
    testCase.SetMask.setBit(RELAY_MONOSTABLE_300_1, false);     // switch MONOSTABLE_300_1 -> 0
    testCase.EnableMask.setBit(RELAY_MONOSTABLE_600_1, true);
    testCase.SetMask.setBit(RELAY_MONOSTABLE_600_1, false);     // switch MONOSTABLE_600_1 -> 0
    testCase.expectedLogicalCurrentMask.setBit(RELAY_MONOSTABLE_300_1, false);
    testCase.expectedLogicalCurrentMask.setBit(RELAY_MONOSTABLE_600_1, false);
    // init compare masks
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT);
    expectedData.clear();
    // low layer callback 1
    resultEnableMask.setBit(PIN_MONOSTABLE_300_1, true);
    resultSetMask.setBit(PIN_MONOSTABLE_300_1, false);
    resultEnableMask.setBit(PIN_MONOSTABLE_600_1, true);
    resultSetMask.setBit(PIN_MONOSTABLE_600_1, false);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 0));
    // add testcase
    testCase.expectedData = expectedData;
    testCases.append(testCase);

    /* define test case */
    testCase.Description = "Monostable300+600->1";
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.bForce = false;
    testCase.bSetMasksBitByBit = false;
    testCase.lowLayerUnblockedDelayMs = 0; // blocked
    testCase.expectedFinalDelay = 600; // monostable max(300,600)
    testCase.EnableMask.setBit(RELAY_MONOSTABLE_300_1, true);
    testCase.SetMask.setBit(RELAY_MONOSTABLE_300_1, true);     // switch MONOSTABLE_300_1 -> 1
    testCase.EnableMask.setBit(RELAY_MONOSTABLE_600_1, true);
    testCase.SetMask.setBit(RELAY_MONOSTABLE_600_1, true);     // switch MONOSTABLE_600_1 -> 1
    testCase.expectedLogicalCurrentMask.setBit(RELAY_MONOSTABLE_300_1, true);
    testCase.expectedLogicalCurrentMask.setBit(RELAY_MONOSTABLE_600_1, true);
    // init compare masks
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT);
    expectedData.clear();
    // low layer callback 1
    resultEnableMask.setBit(PIN_MONOSTABLE_300_1, true);
    resultSetMask.setBit(PIN_MONOSTABLE_300_1, true);
    resultEnableMask.setBit(PIN_MONOSTABLE_600_1, true);
    resultSetMask.setBit(PIN_MONOSTABLE_600_1, true);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 0));
    // add testcase
    testCase.expectedData = expectedData;
    testCases.append(testCase);

    /* define test case */
    testCase.Description = "Monostable300+600->0";
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.bForce = false;
    testCase.bSetMasksBitByBit = false;
    testCase.lowLayerUnblockedDelayMs = 0; // blocked
    testCase.expectedFinalDelay = 600; // monostable max(300,600)
    testCase.EnableMask.setBit(RELAY_MONOSTABLE_300_1, true);
    testCase.SetMask.setBit(RELAY_MONOSTABLE_300_1, false);     // switch MONOSTABLE_300_1 -> 0
    testCase.EnableMask.setBit(RELAY_MONOSTABLE_600_1, true);
    testCase.SetMask.setBit(RELAY_MONOSTABLE_600_1, false);     // switch MONOSTABLE_600_1 -> 0
    testCase.expectedLogicalCurrentMask.setBit(RELAY_MONOSTABLE_300_1, false);
    testCase.expectedLogicalCurrentMask.setBit(RELAY_MONOSTABLE_600_1, false);
    // init compare masks
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT);
    expectedData.clear();
    // low layer callback 1
    resultEnableMask.setBit(PIN_MONOSTABLE_300_1, true);
    resultSetMask.setBit(PIN_MONOSTABLE_300_1, false);
    resultEnableMask.setBit(PIN_MONOSTABLE_600_1, true);
    resultSetMask.setBit(PIN_MONOSTABLE_600_1, false);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 0));
    // add testcase
    testCase.expectedData = expectedData;
    testCases.append(testCase);

    /* define test case */
    testCase.Description = "No OP: Monostable300+600->0 Bistable 50ms->1 / 100ms->0";
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.bForce = false;
    testCase.bSetMasksBitByBit = false;
    testCase.lowLayerUnblockedDelayMs = 0; // blocked
    testCase.expectedFinalDelay = 0; // no OP
    testCase.EnableMask.setBit(RELAY_MONOSTABLE_300_1, true);
    testCase.SetMask.setBit(RELAY_MONOSTABLE_300_1, false);     // switch MONOSTABLE_300_1 -> 0
    testCase.EnableMask.setBit(RELAY_MONOSTABLE_600_1, true);
    testCase.SetMask.setBit(RELAY_MONOSTABLE_600_1, false);     // switch MONOSTABLE_600_1 -> 0
    testCase.EnableMask.setBit(RELAY_BISTABLE_100_1, true);
    testCase.SetMask.setBit(RELAY_BISTABLE_100_1, false);     // switch RELAY_BISTABLE_100_1 -> 0
    testCase.EnableMask.setBit(RELAY_BISTABLE_50_1, true);
    testCase.SetMask.setBit(RELAY_BISTABLE_50_1, true);     // switch RELAY_BISTABLE_50_1 -> 1
    // we don't expect callbacks here
    expectedData.clear();
    // add testcase
    testCase.expectedData = expectedData;
    testCases.append(testCase);

    /* define test case */
    testCase.Description = "Force+Serial all relays -> 0";
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(true,LOGICAL_RELAY_COUNT); // init
    testCase.bForce = true;
    testCase.bSetMasksBitByBit = true;
    testCase.lowLayerUnblockedDelayMs = 0; // blocked
    testCase.expectedFinalDelay = 600-100; // monostable max - bistable max
    testCase.expectedLogicalCurrentMask.fill(false,LOGICAL_RELAY_COUNT);
    // init compare masks
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT);
    expectedData.clear();
    // low layer callback 1
    resultEnableMask.setBit(PIN_BISTABLE_100_1_OFF, true);
    resultSetMask.setBit(PIN_BISTABLE_100_1_OFF, true);
    resultEnableMask.setBit(PIN_BISTABLE_100_2_OFF, true);
    resultSetMask.setBit(PIN_BISTABLE_100_2_OFF, true);
    resultEnableMask.setBit(PIN_BISTABLE_50_1_OFF, true);
    resultSetMask.setBit(PIN_BISTABLE_50_1_OFF, true);
    resultEnableMask.setBit(PIN_BISTABLE_50_2_OFF, true);
    resultSetMask.setBit(PIN_BISTABLE_50_2_OFF, false); // inverted
    resultEnableMask.setBit(PIN_MONOSTABLE_300_1, true);
    resultSetMask.setBit(PIN_MONOSTABLE_300_1, false);
    resultEnableMask.setBit(PIN_MONOSTABLE_300_2, true);
    resultSetMask.setBit(PIN_MONOSTABLE_300_2, true); // inverted
    resultEnableMask.setBit(PIN_MONOSTABLE_600_1, true);
    resultSetMask.setBit(PIN_MONOSTABLE_600_1, false);
    resultEnableMask.setBit(PIN_MONOSTABLE_600_2, true);
    resultSetMask.setBit(PIN_MONOSTABLE_600_2, false);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 0));
    // low layer callback 2
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT);
    resultEnableMask.setBit(PIN_BISTABLE_50_1_OFF, true);
    resultSetMask.setBit(PIN_BISTABLE_50_1_OFF, false);
    resultEnableMask.setBit(PIN_BISTABLE_50_2_OFF, true);
    resultSetMask.setBit(PIN_BISTABLE_50_2_OFF, true); // inverted
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 50));
    // low layer callback 3
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT);
    resultEnableMask.setBit(PIN_BISTABLE_100_1_OFF, true);
    resultEnableMask.setBit(PIN_BISTABLE_100_2_OFF, true);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 100-50));
    // add testcase
    testCase.expectedData = expectedData;
    testCases.append(testCase);

    /* define test case */
    testCase.Description = "Serial all relays -> 1";
    testCase.SetMask.fill(true,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(true,LOGICAL_RELAY_COUNT); // init
    testCase.bForce = false;
    testCase.bSetMasksBitByBit = true;
    testCase.lowLayerUnblockedDelayMs = 0; // blocked
    testCase.expectedFinalDelay = 600-100; // monostable max - bistable max
    testCase.expectedLogicalCurrentMask.fill(true,LOGICAL_RELAY_COUNT);
    // init compare masks
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT);
    expectedData.clear();
    // low layer callback 1
    resultEnableMask.setBit(PIN_BISTABLE_100_1_ON, true);
    resultSetMask.setBit(PIN_BISTABLE_100_1_ON, true);
    resultEnableMask.setBit(PIN_BISTABLE_100_2_ON, true);
    resultSetMask.setBit(PIN_BISTABLE_100_2_ON, false); // inverted
    resultEnableMask.setBit(PIN_BISTABLE_50_1_ON, true);
    resultSetMask.setBit(PIN_BISTABLE_50_1_ON, true);
    resultEnableMask.setBit(PIN_BISTABLE_50_2_ON, true);
    resultSetMask.setBit(PIN_BISTABLE_50_2_ON, true);
    resultEnableMask.setBit(PIN_MONOSTABLE_300_1, true);
    resultSetMask.setBit(PIN_MONOSTABLE_300_1, true);
    resultEnableMask.setBit(PIN_MONOSTABLE_300_2, true);
    resultSetMask.setBit(PIN_MONOSTABLE_300_2, false); // inverted
    resultEnableMask.setBit(PIN_MONOSTABLE_600_1, true);
    resultSetMask.setBit(PIN_MONOSTABLE_600_1, true);
    resultEnableMask.setBit(PIN_MONOSTABLE_600_2, true);
    resultSetMask.setBit(PIN_MONOSTABLE_600_2, true);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 0));
    // low layer callback 2
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT);
    resultEnableMask.setBit(PIN_BISTABLE_50_1_ON, true);
    resultSetMask.setBit(PIN_BISTABLE_50_1_ON, false);
    resultEnableMask.setBit(PIN_BISTABLE_50_2_ON, true);
    resultSetMask.setBit(PIN_BISTABLE_50_2_ON, false);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 50));
    // low layer callback 3
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT);
    resultEnableMask.setBit(PIN_BISTABLE_100_1_ON, true);
    resultEnableMask.setBit(PIN_BISTABLE_100_2_ON, true);
    resultSetMask.setBit(PIN_BISTABLE_100_2_ON, true); // inverted
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 100-50));
    // add testcase
    testCase.expectedData = expectedData;
    testCases.append(testCase);

    /* define test case */
    testCase.Description = "Unblocked bistable 100ms->0";
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.expectedLogicalCurrentMask.setBit(RELAY_BISTABLE_100_1, false);
    testCase.bForce = false;
    testCase.bSetMasksBitByBit = false;
    testCase.lowLayerUnblockedDelayMs = 100; // unblocked
    testCase.expectedFinalDelay = 0 + testCase.lowLayerUnblockedDelayMs; // bistable only
    testCase.EnableMask.setBit(RELAY_BISTABLE_100_1, true);
    testCase.SetMask.setBit(RELAY_BISTABLE_100_1, false);
    // init compare masks
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT);
    expectedData.clear();
    // low layer callback 1
    resultEnableMask.setBit(PIN_BISTABLE_100_1_OFF, true);
    resultSetMask.setBit(PIN_BISTABLE_100_1_OFF, true);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 0));
    // low layer callback 2
    resultSetMask.setBit(PIN_BISTABLE_100_1_OFF, false);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 100+testCase.lowLayerUnblockedDelayMs));
    // add testcase
    testCase.expectedData = expectedData;
    testCases.append(testCase);

    /* define test case */
    testCase.Description = "Unblocked bistable 50ms->0";
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.expectedLogicalCurrentMask.setBit(RELAY_BISTABLE_50_1, false);
    testCase.bForce = false;
    testCase.bSetMasksBitByBit = false;
    testCase.lowLayerUnblockedDelayMs = 100; // unblocked
    testCase.expectedFinalDelay = 0 + testCase.lowLayerUnblockedDelayMs; // bistable only
    testCase.EnableMask.setBit(RELAY_BISTABLE_50_1, true);
    testCase.SetMask.setBit(RELAY_BISTABLE_50_1, false);
    // init compare masks
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT);
    expectedData.clear();
    // low layer callback 1
    resultEnableMask.setBit(PIN_BISTABLE_50_1_OFF, true);
    resultSetMask.setBit(PIN_BISTABLE_50_1_OFF, true);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 0));
    // low layer callback 2
    resultSetMask.setBit(PIN_BISTABLE_50_1_OFF, false);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 50+testCase.lowLayerUnblockedDelayMs));
    // add testcase
    testCase.expectedData = expectedData;
    testCases.append(testCase);
}

static void appendTestCasesSequencer(QList<TTestCase> &testCases)
{
    QList<TExpectedLowLayerData> expectedData;
    TTestCase testCase;
    QBitArray resultEnableMask;
    QBitArray resultSetMask;

    /* define test case */
    testCase.Description = "Transparent->1";
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.expectedLogicalCurrentMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.bForce = false;
    testCase.bSetMasksBitByBit = false;
    testCase.lowLayerUnblockedDelayMs = 0; // blocked
    testCase.expectedFinalDelay = 100;
    testCase.EnableMask.setBit(RELAY_TRANSPARENT_1, true);
    testCase.SetMask.setBit(RELAY_TRANSPARENT_1, true);
    // init compare masks
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT_SEQUENCER);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT_SEQUENCER);
    expectedData.clear();
    // low layer callback 1
    resultEnableMask.setBit(PIN_TRANSPARENT_1, true);
    resultSetMask.setBit(PIN_TRANSPARENT_1, true);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 0));
    // add testcase
    testCase.expectedData = expectedData;
    testCases.append(testCase);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QElapsedTimer timerElapsedApplication;
    timerElapsedApplication.start();

    QCommandLineParser parser;
    parser.setApplicationDescription("Relay Mapper Test");
    parser.addHelpOption();
    parser.process(a);

    int currTestCase = 0;
    int currCallback = 0;
    QElapsedTimer timerElapsedTestCase;

    // Timers are not that precise so accept some deviation
    const double maxDelayDeviation = 0.1; /* 10 % */

    // Setup test cases
    QList<TTestCase> testCases;
    appendTestCasesMapper(testCases);
    int relayMapperTestCases = testCases.count();
    appendTestCasesSequencer(testCases);
    int relaySequencerTestCases = testCases.count() - relayMapperTestCases;

    bool bSingleTestError = false;
    bool bTotalTestError = false;

    // Timer to simulate unblocked low layer
    QTimer timerForUnblocked;
    timerForUnblocked.setSingleShot(true);

    /* setup relay mapper */
    QRelayMapper relayMapper;
    initRelayMapperSetup();
    QObject::connect(&timerForUnblocked, &QTimer::timeout, &relayMapper, &QRelayMapper::onLowLayerIdle);
    relayMapper.setupCallbackLowLayerBusy([&]()
    {
        return timerForUnblocked.isActive();
    });

    // Relay mapper callback
    RelayStartLowLayerSwitchFunction CallbackStartLowLayerSwitch = [&] (
              const QBitArray& EnableMask,
              const QBitArray& SetMask,
              const QObject* pCaller)
    {   // relay callback begin
        Q_UNUSED(pCaller)
        qint64 elapsed = timerElapsedTestCase.elapsed();
        timerElapsedTestCase.start();

        if(currTestCase<testCases.count())
        {
            qInfo() << "Callback:" << currCallback+1;
            if(currCallback < testCases[currTestCase].expectedData.size())
            {
                int delaySinceLast = testCases[currTestCase].expectedData[currCallback].delaySinceLast;
                if(delaySinceLast != 0)
                {
                    // Check delay deviation
                    double expectedDelay = delaySinceLast;
                    double actualDelay = elapsed;
                    double deviation = (actualDelay-expectedDelay)/expectedDelay;
                    if(fabs(deviation) > maxDelayDeviation)
                    {
                        bSingleTestError = true;
                        qWarning() <<  "Error!!! delay out of limit:" << elapsed << "expected:" << delaySinceLast;
                    }
                    else
                        qInfo() <<  "Delay since last:" << elapsed << "expected:" << delaySinceLast;
                }
                // TDB: limits check for 0-delay case?
                else
                    qInfo() <<  "Delay since last:" << elapsed << "expected:" << delaySinceLast;

                QBitArray expectedEnableMask = testCases[currTestCase].expectedData[currCallback].EnableMask;
                QBitArray expectedSetMask = testCases[currTestCase].expectedData[currCallback].SetMask;
                if(expectedEnableMask == EnableMask)
                    qInfo() << "EnableMask:" << EnableMask << "OK";
                else
                {
                    bSingleTestError = true;
                    qWarning() << "Error!!! EnableMask:" << EnableMask << "expected" << expectedEnableMask;
                }
                if(expectedSetMask == SetMask)
                    qInfo() << "SetMask:   " << SetMask << "OK";
                else
                {
                    bSingleTestError = true;
                    qWarning() << "Error!!! SetMask:   " << SetMask << "expected" << expectedSetMask;
                }
            }
            else
            {
                bSingleTestError = true;
                qWarning() <<  "Error!!! Unconfigured response";
            }
        }
        else
        {
            bSingleTestError = true;
            qWarning() <<  "Error!!! Test case out of limit";
        }
        currCallback++;
        if(testCases[currTestCase].lowLayerUnblockedDelayMs==0)
            return false;
        else
        {
            timerForUnblocked.start(testCases[currTestCase].lowLayerUnblockedDelayMs);
            return true;
        }
    };  // relay callback end

    relayMapper.setup(LOGICAL_RELAY_COUNT,
                      arrRelayMapperSetup,
                      sliceTimerPeriod,
                      CallbackStartLowLayerSwitch);

    // Relay sequencer
    QRelaySequencer relaySequencer;
    initRelayMapperSetupSequencer();
    relaySequencer.SetLowLayer(&relayMapper);

    // run all test cases in singleshot timerElapsedTestCase -> we need working evenloop
    QTimer::singleShot(300,[&]
                       ()
    {   // singleshot timer callback begin
        int idleCountMapper = 0;
        int idleCountSequencer = 0;
        // Simple idle counter for mapper
        QObject::connect(
            &relayMapper, &QRelayMapper::idle,
            [&]()
        {
            // do not count upper layer signals
            if(currTestCase<relayMapperTestCases)
                idleCountMapper++;
        });
        // Simple idle counter for sequencer
        QObject::connect(
            &relaySequencer, &QRelaySequencer::idle,
            [&]()
        {
            idleCountSequencer++;
        });

        // loop all test cases
        for(;currTestCase<testCases.count(); currTestCase++)
        {
            // For the case that signals are not emitted / handled the application might
            // hang. To catch this we add a timeout
            QTimer::singleShot(10000,[&]
                               ()
            {   // singleshot timer callback begin
                qWarning() << "";
                qWarning("Error!!! Test application seems to hang and is terminated");
                a.exit(-1);
            }); // singleshot timer callback end

            currCallback = 0;
            bSingleTestError = false;
            timerElapsedTestCase.start();

            // run relay layers blocked by helper event loop
            QEventLoop loop;
            QRelayBase *currentLayer;
            if(currTestCase<relayMapperTestCases)
            {
                currentLayer = &relayMapper;
                QObject::connect(currentLayer, &QRelayMapper::idle, &loop, &QEventLoop::quit);
            }
            else
            {
                currentLayer = &relaySequencer;
                QObject::connect(currentLayer, &QRelaySequencer::idle, &loop, &QEventLoop::quit);
            }
            if(currTestCase > 0)
                qInfo() << "";
            qInfo() << "Starting test case:" << testCases[currTestCase].Description;
            if(!testCases[currTestCase].bSetMasksBitByBit)
                currentLayer->startSetMulti(
                            testCases[currTestCase].EnableMask,
                            testCases[currTestCase].SetMask,
                            testCases[currTestCase].bForce);
            else
            {
                qInfo() << "Performing test by bit by bit call";
                for(qint16 logRelay=0;
                    logRelay<testCases[currTestCase].EnableMask.size() && logRelay<currentLayer->getLogicalRelayCount();
                    logRelay++)
                {
                    if(testCases[currTestCase].EnableMask.at(logRelay))
                    {
                        bool bSet = logRelay<testCases[currTestCase].SetMask.size() ? testCases[currTestCase].SetMask.at(logRelay) : false;
                        currentLayer->startSet(logRelay, bSet, testCases[currTestCase].bForce);
                    }
                }
            }
            loop.exec();

            // Check for final delay
            qint64 elapsed = timerElapsedTestCase.elapsed();
            qint64 expectedFinalDelay = testCases[currTestCase].expectedFinalDelay;
            if(expectedFinalDelay)
            {
                // Check delay deviation
                double expectedDelay = expectedFinalDelay;
                double actualDelay = elapsed;
                double deviation = (actualDelay-expectedDelay)/expectedDelay;
                if(fabs(deviation) > maxDelayDeviation)
                {
                    bSingleTestError = true;
                    qWarning() << "Error!!! Final delay out of limit:" << elapsed << "expected:" << expectedFinalDelay;
                }
                else
                    qInfo() << "Final delay:" << elapsed << "expected:" << expectedFinalDelay;
            }
            else
                // TDB: limits check for 0-delay case?
                qInfo() << "Final delay:" << elapsed << "expected:" << expectedFinalDelay;

            // Check if all expected callbacks were called
            int expectedCallbacks = testCases[currTestCase].expectedData.size();
            if(currCallback < expectedCallbacks)
            {
                bSingleTestError = true;
                qWarning() << "Error!!! To few callback called expected:" << expectedCallbacks << "received:" << currCallback;
            }
            // Check if the logical state is as expected
            QBitArray expectedLogicalMask = testCases[currTestCase].expectedLogicalCurrentMask;
            QBitArray actualLogicalMask = currentLayer->getLogicalRelayState();
            if(actualLogicalMask == expectedLogicalMask)
                qInfo() << "Actual mask logical:" << actualLogicalMask;
            else
            {
                bSingleTestError = true;
                qWarning() << "Error!!! Actual mask logical expected:" << expectedLogicalMask << "reported:" << actualLogicalMask;
            }
            if(bSingleTestError)
                bTotalTestError = true;
        } // test case loop end

        // correct number of idles received?
        qInfo() << "";
        if(idleCountMapper==relayMapperTestCases)
            qInfo() << "Mapper idle signals:" << idleCountMapper << "OK";
        else
        {
            bTotalTestError = true;
            qWarning() << "Error!!! Mapper Idle signals" << idleCountMapper << "expected:" << relayMapperTestCases;
        }
        if(idleCountSequencer==relaySequencerTestCases)
            qInfo() << "Sequencer idle signals:" << idleCountSequencer << "OK";
        else
        {
            bTotalTestError = true;
            qWarning() << "Error!!! Sequencer Idle signals" << idleCountSequencer << "expected:" << relaySequencerTestCases;
        }

        qInfo() << "";
        qInfo() << "All tests took" << timerElapsedApplication.elapsed()/1000 << "s";
        // End message
        if(!bTotalTestError)
        {
            qInfo() << "All tests passed.";
            a.exit(0);
        }
        else
        {
            qWarning() << "Error(s) occured!!!";
            a.exit(-1);
        }
    }); // singleshot timer callback end

    return a.exec();
}
