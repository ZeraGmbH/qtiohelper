#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTimer>
#include <QtDebug>
#include <QElapsedTimer>
#include <math.h>
#include "../../relay-mapper/relay-mapper.h"

enum enLogicalRelayDefinition
{
    RELAY_BISTABLE_500_1 = 0,
    RELAY_BISTABLE_500_2,
    RELAY_BISTABLE_200_1,
    RELAY_BISTABLE_200_2,

    RELAY_MONOSTABLE_300_1,
    RELAY_MONOSTABLE_300_2,
    RELAY_MONOSTABLE_600_1,
    RELAY_MONOSTABLE_600_2,

    LOGICAL_RELAY_COUNT
};

enum enPhysPins
{
    PIN_BISTABLE_500_1_ON = 0,
    PIN_BISTABLE_500_1_OFF,
    PIN_BISTABLE_500_2_ON,
    PIN_BISTABLE_500_2_OFF,
    PIN_BISTABLE_200_1_ON,
    PIN_BISTABLE_200_1_OFF,
    PIN_BISTABLE_200_2_ON,
    PIN_BISTABLE_200_2_OFF,

    PIN_MONOSTABLE_300_1,
    PIN_MONOSTABLE_300_2,
    PIN_MONOSTABLE_600_1,
    PIN_MONOSTABLE_600_2,

    PHYSICAL_PIN_COUNT
};

struct TLogicalRelayEntry arrRelayMapperSetup[LOGICAL_RELAY_COUNT];

static void InitRelayMapperSetup()
{
    TLogicalRelayEntry* pEntry;

    pEntry = arrRelayMapperSetup + RELAY_BISTABLE_500_1;
    pEntry->ui16OnPosition  = PIN_BISTABLE_500_1_ON;
    pEntry->ui16OffPosition = PIN_BISTABLE_500_1_OFF;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(true /*bistable*/, false/*on not-inverted*/, false /* off nit inverted*/),
    pEntry->ui8OnTime = 5; // 500ms

    pEntry = arrRelayMapperSetup + RELAY_BISTABLE_500_2;
    pEntry->ui16OnPosition  = PIN_BISTABLE_500_2_ON;
    pEntry->ui16OffPosition = PIN_BISTABLE_500_2_OFF;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(true /*bistable*/, false/*on not-inverted*/, false /* off nit inverted*/),
    pEntry->ui8OnTime = 5; // 500ms

    pEntry = arrRelayMapperSetup + RELAY_BISTABLE_200_1;
    pEntry->ui16OnPosition  = PIN_BISTABLE_200_1_ON;
    pEntry->ui16OffPosition = PIN_BISTABLE_200_1_OFF;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(true /*bistable*/, false/*on not-inverted*/, false /* off nit inverted*/),
    pEntry->ui8OnTime = 2; // 200ms

    pEntry = arrRelayMapperSetup + RELAY_BISTABLE_200_2;
    pEntry->ui16OnPosition  = PIN_BISTABLE_200_2_ON;
    pEntry->ui16OffPosition = PIN_BISTABLE_200_2_OFF;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(true /*bistable*/, false/*on not-inverted*/, false /* off nit inverted*/),
    pEntry->ui8OnTime = 2; // 200ms

    pEntry = arrRelayMapperSetup + RELAY_MONOSTABLE_300_1;
    pEntry->ui16OnPosition  = PIN_MONOSTABLE_300_1;
    pEntry->ui16OffPosition = PIN_MONOSTABLE_300_1;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable*/, false/*on not-inverted*/, false /* off nit inverted*/),
    pEntry->ui8OnTime = 3; // 300ms

    pEntry = arrRelayMapperSetup + RELAY_MONOSTABLE_300_2;
    pEntry->ui16OnPosition  = PIN_MONOSTABLE_300_2;
    pEntry->ui16OffPosition = PIN_MONOSTABLE_300_2;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable*/, false/*on not-inverted*/, false /* off nit inverted*/),
    pEntry->ui8OnTime = 3; // 300ms

    pEntry = arrRelayMapperSetup + RELAY_MONOSTABLE_600_1;
    pEntry->ui16OnPosition  = PIN_MONOSTABLE_600_1;
    pEntry->ui16OffPosition = PIN_MONOSTABLE_600_1;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable*/, false/*on not-inverted*/, false /* off nit inverted*/),
    pEntry->ui8OnTime = 6; // 600ms

    pEntry = arrRelayMapperSetup + RELAY_MONOSTABLE_600_2;
    pEntry->ui16OnPosition  = PIN_MONOSTABLE_600_2;
    pEntry->ui16OffPosition = PIN_MONOSTABLE_600_2;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable*/, false/*on not-inverted*/, false /* off nit inverted*/),
    pEntry->ui8OnTime = 6; // 600ms
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
    QList<TExpectedLowLayerData> expectedData;
};


static void initTestCases(QList<TTestCase> &testCases)
{
    QList<TExpectedLowLayerData> expectedData;
    TTestCase testCase;
    QBitArray resultEnableMask;
    QBitArray resultSetMask;

    /* define test case */
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.expectedLogicalCurrentMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.Description = "Forced bistable 500ms->0";
    testCase.bForce = true;
    testCase.expectedFinalDelay = 0; // bistable only
    testCase.EnableMask.setBit(RELAY_BISTABLE_500_1, true);
    testCase.SetMask.setBit(RELAY_BISTABLE_500_1, false);    // force RELAY_BISTABLE_200_1 -> 0
    // init compare masks
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT);
    expectedData.clear();
    // switch 1
    resultEnableMask.setBit(PIN_BISTABLE_500_1_OFF, true);
    resultSetMask.setBit(PIN_BISTABLE_500_1_OFF, true);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 0));
    // switch 2
    resultSetMask.setBit(PIN_BISTABLE_500_1_OFF, false);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 500));
    // add testcase
    testCase.expectedData = expectedData;
    testCases.append(testCase);

    /* define test case */
    testCase.Description = "Forced bistable 500ms->0 (2nd time: check force)";
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.bForce = true;
    testCase.EnableMask.setBit(RELAY_BISTABLE_500_1, true);
    testCase.SetMask.setBit(RELAY_BISTABLE_500_1, false);    // force RELAY_BISTABLE_200_1 -> 0
    // init compare masks
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT);
    expectedData.clear();
    // switch 1
    resultEnableMask.setBit(PIN_BISTABLE_500_1_OFF, true);
    resultSetMask.setBit(PIN_BISTABLE_500_1_OFF, true);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 0));
    // switch 2
    resultSetMask.setBit(PIN_BISTABLE_500_1_OFF, false);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 500));
    // add testcase
    testCase.expectedData = expectedData;
    testCases.append(testCase);

    /* define test case */
    testCase.Description = "Forced bistable 200ms->0 + ignore 500ms->1";
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.bForce = true;
    testCase.expectedFinalDelay = 0; // bistable only
    testCase.EnableMask.setBit(RELAY_BISTABLE_200_1, true);
    testCase.SetMask.setBit(RELAY_BISTABLE_200_1, false); // force RELAY_BISTABLE_200_1 -> 0
    testCase.EnableMask.setBit(RELAY_BISTABLE_500_1, false);
    testCase.SetMask.setBit(RELAY_BISTABLE_500_1, true);  // ignore RELAY_BISTABLE_500_1 -> 1 (ignored)
    expectedData.clear();
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT);
    // switch 1
    resultEnableMask.setBit(PIN_BISTABLE_200_1_OFF, true);
    resultSetMask.setBit(PIN_BISTABLE_200_1_OFF, true);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 0));
    // switch 2
    resultSetMask.setBit(PIN_BISTABLE_200_1_OFF, false);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 200));
    // add testcase
    testCase.expectedData = expectedData;
    testCases.append(testCase);

    /* define test case */
    testCase.Description = "Bistable 200ms->1 / 500ms->1";
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.bForce = false;
    testCase.expectedFinalDelay = 0; // bistable only
    testCase.EnableMask.setBit(RELAY_BISTABLE_500_1, true);
    testCase.SetMask.setBit(RELAY_BISTABLE_500_1, true);     // switch RELAY_BISTABLE_500_1 -> 1
    testCase.EnableMask.setBit(RELAY_BISTABLE_200_1, true);
    testCase.SetMask.setBit(RELAY_BISTABLE_200_1, true);     // switch RELAY_BISTABLE_200_1 -> 1
    testCase.expectedLogicalCurrentMask.setBit(RELAY_BISTABLE_500_1, true);
    testCase.expectedLogicalCurrentMask.setBit(RELAY_BISTABLE_200_1, true);
    // init compare masks
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT);
    expectedData.clear();
    // switch 1
    resultEnableMask.setBit(PIN_BISTABLE_500_1_ON, true);
    resultSetMask.setBit(PIN_BISTABLE_500_1_ON, true);
    resultEnableMask.setBit(PIN_BISTABLE_200_1_ON, true);
    resultSetMask.setBit(PIN_BISTABLE_200_1_ON, true);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 0));
    // switch 2
    resultEnableMask.setBit(PIN_BISTABLE_500_1_ON, false);
    resultSetMask.setBit(PIN_BISTABLE_500_1_ON, false);
    resultSetMask.setBit(PIN_BISTABLE_200_1_ON, false);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 200));
    // switch 3
    resultEnableMask.setBit(PIN_BISTABLE_500_1_ON, true);
    resultSetMask.setBit(PIN_BISTABLE_500_1_ON, false);
    resultEnableMask.setBit(PIN_BISTABLE_200_1_ON, false);
    resultSetMask.setBit(PIN_BISTABLE_200_1_ON, false);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 500-200));
    // add testcase
    testCase.expectedData = expectedData;
    testCases.append(testCase);

    /* define test case */
    testCase.Description = "Bistable 200ms->1 (no change) / 500ms-> 0";
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.bForce = false;
    testCase.expectedFinalDelay = 0; // bistable only
    testCase.EnableMask.setBit(RELAY_BISTABLE_500_1, true);
    testCase.SetMask.setBit(RELAY_BISTABLE_500_1, false);     // switch RELAY_BISTABLE_500_1 -> 0
    testCase.EnableMask.setBit(RELAY_BISTABLE_200_1, true);
    testCase.SetMask.setBit(RELAY_BISTABLE_200_1, true);     // switch RELAY_BISTABLE_200_1 -> 1 (no change)
    testCase.expectedLogicalCurrentMask.setBit(RELAY_BISTABLE_500_1, false);
    // init compare masks
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT);
    expectedData.clear();
    // switch 1
    resultEnableMask.setBit(PIN_BISTABLE_500_1_OFF, true);
    resultSetMask.setBit(PIN_BISTABLE_500_1_OFF, true);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 0));
    // switch 2
    resultSetMask.setBit(PIN_BISTABLE_500_1_OFF, false);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 500));
    // add testcase
    testCase.expectedData = expectedData;
    testCases.append(testCase);

    /* define test case */
    testCase.Description = "Force Monostable_300+600->0";
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.bForce = true;
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
    // switch 1
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
    // switch 1
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
    // switch 1
    resultEnableMask.setBit(PIN_MONOSTABLE_300_1, true);
    resultSetMask.setBit(PIN_MONOSTABLE_300_1, false);
    resultEnableMask.setBit(PIN_MONOSTABLE_600_1, true);
    resultSetMask.setBit(PIN_MONOSTABLE_600_1, false);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 0));
    // add testcase
    testCase.expectedData = expectedData;
    testCases.append(testCase);

    /* define test case */
    testCase.Description = "No OP: Monostable300+600->0 Bistable 200ms->1 / 500ms->0";
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.bForce = false;
    testCase.expectedFinalDelay = 0; // no OP
    testCase.EnableMask.setBit(RELAY_MONOSTABLE_300_1, true);
    testCase.SetMask.setBit(RELAY_MONOSTABLE_300_1, false);     // switch MONOSTABLE_300_1 -> 0
    testCase.EnableMask.setBit(RELAY_MONOSTABLE_600_1, true);
    testCase.SetMask.setBit(RELAY_MONOSTABLE_600_1, false);     // switch MONOSTABLE_600_1 -> 0
    testCase.EnableMask.setBit(RELAY_BISTABLE_500_1, true);
    testCase.SetMask.setBit(RELAY_BISTABLE_500_1, false);     // switch RELAY_BISTABLE_500_1 -> 0
    testCase.EnableMask.setBit(RELAY_BISTABLE_200_1, true);
    testCase.SetMask.setBit(RELAY_BISTABLE_200_1, true);     // switch RELAY_BISTABLE_200_1 -> 1
    // we don't expect callbacks
    expectedData.clear();
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

    // Setup test cases
    const double maxDelayDeviation = 0.1; /* 10 % */
    QList<TTestCase> testCases;
    initTestCases(testCases);


    bool bSingleTestError = false;
    bool bTotalTestError = false;
    /* setup relay mapper */
    QRelayMapper relayMapper;
    InitRelayMapperSetup();
    relayMapper.setup(LOGICAL_RELAY_COUNT,
                      arrRelayMapperSetup,
                      100,  // 100ms
                      [&]
                      (
                        const QBitArray& EnableMask,
                        const QBitArray& SetMask,
                        const QObject* pCaller
                      )
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
                        qWarning() <<  "Error!!! delay out of limit expected:" << delaySinceLast << "actual" << elapsed;
                    }
                }
                // TDB: limits check for 0-delay case?
                else
                    qInfo() <<  "Delay since last:" << elapsed;

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
        return false;
    }); // relay callback end


    // run all test cases in singleshot timerElapsedTestCase -> we need working evenloop
    QTimer::singleShot(300,[&]
                       ()
    {   // singleshot timer callback begin
        int idleCount = 0;
        // Simple idle counter
        QObject::connect(
            &relayMapper, &QRelayMapper::idle,
            [&]() { idleCount++; }
        );
        for(;currTestCase<testCases.count(); currTestCase++)
        {
            currCallback = 0;
            bSingleTestError = false;
            timerElapsedTestCase.start();
            QEventLoop loop;
            QObject::connect(&relayMapper, &QRelayMapper::idle, &loop, &QEventLoop::quit);
            if(currTestCase > 0)
                qInfo() << "";
            qInfo() << "Starting test case:" << testCases[currTestCase].Description;
            relayMapper.startSetMulti(
                        testCases[currTestCase].EnableMask,
                        testCases[currTestCase].SetMask,
                        testCases[currTestCase].bForce);
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
                    qInfo() << "Final delay:" << elapsed;
            }
            else
                // TDB: limits check for 0-delay case?
                qInfo() << "Final delay:" << elapsed;

            // Check if all expected callbacks were called
            int expectedCallbacks = testCases[currTestCase].expectedData.size();
            if(currCallback < expectedCallbacks)
            {
                bSingleTestError = true;
                qWarning() << "Error!!! To few callback called expected:" << expectedCallbacks << "received:" << currCallback;
            }
            // Check if the logical state is as expected
            QBitArray expectedLogicalMask = testCases[currTestCase].expectedLogicalCurrentMask;
            QBitArray actualLogicalMask = relayMapper.getLogicalRelayState();
            if(actualLogicalMask == expectedLogicalMask)
                qInfo() << "Actual mask logical:" << actualLogicalMask;
            else
            {
                bSingleTestError = true;
                qWarning() << "Error!!! Actual mask logical expected:" << expectedLogicalMask << "reported:" << actualLogicalMask;
            }
            if(bSingleTestError)
                bTotalTestError = true;
        }
        // correct number of idles received?
        if(idleCount==testCases.count())
        {
            qInfo() << "";
            qInfo() << "Idle signals received:" << idleCount;
        }
        else
        {
            bTotalTestError = true;
            qWarning() << "Error!!! Idle signals received" << idleCount << "expected:" << testCases.count();
        }
        qInfo() << "All tests took" << timerElapsedApplication.elapsed()/1000 << "s";
        // End message
        if(!bTotalTestError)
        {
            qInfo() << "";
            qInfo() << "All tests passed.";
            a.exit(0);
        }
        else
        {
            qWarning() << "";
            qWarning() << "Error(s) occured!!!";
            a.exit(-1);
        }
    }); // singleshot timer callback end

    return a.exec();
}
