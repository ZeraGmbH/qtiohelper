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

    RELAY_MONOSTABLE_1,
    RELAY_MONOSTABLE_2,
    RELAY_MONOSTABLE_3,
    RELAY_MONOSTABLE_4,

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

    PIN_MONOSTABLE_1,
    PIN_MONOSTABLE_2,
    PIN_MONOSTABLE_3,
    PIN_MONOSTABLE_4,

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

    pEntry = arrRelayMapperSetup + RELAY_MONOSTABLE_1;
    pEntry->ui16OnPosition  = PIN_MONOSTABLE_1;
    pEntry->ui16OffPosition = PIN_MONOSTABLE_1;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable*/, false/*on not-inverted*/, false /* off nit inverted*/),
    pEntry->ui8OnTime = 5; // 500ms

    pEntry = arrRelayMapperSetup + RELAY_MONOSTABLE_2;
    pEntry->ui16OnPosition  = PIN_MONOSTABLE_2;
    pEntry->ui16OffPosition = PIN_MONOSTABLE_2;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable*/, false/*on not-inverted*/, false /* off nit inverted*/),
    pEntry->ui8OnTime = 5; // 500ms

    pEntry = arrRelayMapperSetup + RELAY_MONOSTABLE_3;
    pEntry->ui16OnPosition  = PIN_MONOSTABLE_3;
    pEntry->ui16OffPosition = PIN_MONOSTABLE_3;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable*/, false/*on not-inverted*/, false /* off nit inverted*/),
    pEntry->ui8OnTime = 5; // 500ms

    pEntry = arrRelayMapperSetup + RELAY_MONOSTABLE_4;
    pEntry->ui16OnPosition  = PIN_MONOSTABLE_4;
    pEntry->ui16OffPosition = PIN_MONOSTABLE_4;
    pEntry->ui8Flags = RELAY_PHYS_FLAGS(false /*monostable*/, false/*on not-inverted*/, false /* off nit inverted*/),
    pEntry->ui8OnTime = 5; // 500ms
}

struct TExpectedLowLayerData
{
    TExpectedLowLayerData(QBitArray &EnableMask_, QBitArray &SetMask_,  int msSinceLast_)
    {
        EnableMask = EnableMask_;
        SetMask = SetMask_;
        msSinceLast = msSinceLast_;
    }
    QBitArray EnableMask;
    QBitArray SetMask;
    int msSinceLast;
};

struct TTestCase
{
    /*TTestCase(QString &Description_, QBitArray &EnableMask_, QBitArray &SetMask_, bool &bForce_, QList<TExpectedLowLayerData> &expectedData_)
    {
        Description = Description_;
        EnableMask = EnableMask_;
        SetMask = SetMask_;
        bForce = bForce_;
        expectedData = expectedData_;
    }*/
    QString Description;
    QBitArray EnableMask;
    QBitArray SetMask;
    QBitArray expectedLogicalCurrentMask;
    bool bForce;
    QList<TExpectedLowLayerData> expectedData;
};


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("Relay Mapper Test");
    parser.addHelpOption();

    /*// option for port number
    QCommandLineOption portOption(QStringList() << "p" << "port", "IP-port number", "portnumber");
    portOption.setDefaultValue("5000");
    parser.addOption(portOption);*/

    parser.process(a);

    int currTestCase = 0;
    int currCallback = 0;
    QElapsedTimer timer;
    // Setup test cases

    QList<TTestCase> testCases;
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
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.Description = "Forced bistable 500ms->0 (2nd time: check force)";
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
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.Description = "Forced bistable 200ms->0 + ignore 500ms->1";
    testCase.bForce = true;
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
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.Description = "Bistable 200ms->1 / 500ms->1";
    testCase.bForce = false;
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
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.Description = "Bistable 200ms->1 (no change) / 500ms-> 0";
    testCase.bForce = false;
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
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.Description = "Force Monostable1+3->0";
    testCase.bForce = true;
    testCase.EnableMask.setBit(RELAY_MONOSTABLE_1, true);
    testCase.SetMask.setBit(RELAY_MONOSTABLE_1, false);     // switch MONOSTABLE_1 -> 0
    testCase.EnableMask.setBit(RELAY_MONOSTABLE_3, true);
    testCase.SetMask.setBit(RELAY_MONOSTABLE_3, false);     // switch MONOSTABLE_3 -> 0
    testCase.expectedLogicalCurrentMask.setBit(RELAY_MONOSTABLE_1, false);
    testCase.expectedLogicalCurrentMask.setBit(RELAY_MONOSTABLE_3, false);
    // init compare masks
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT);
    expectedData.clear();
    // switch 1
    resultEnableMask.setBit(PIN_MONOSTABLE_1, true);
    resultSetMask.setBit(PIN_MONOSTABLE_1, false);
    resultEnableMask.setBit(PIN_MONOSTABLE_3, true);
    resultSetMask.setBit(PIN_MONOSTABLE_3, false);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 0));
    // add testcase
    testCase.expectedData = expectedData;
    testCases.append(testCase);

    /* define test case */
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.Description = "Monostable1+3->1";
    testCase.bForce = false;
    testCase.EnableMask.setBit(RELAY_MONOSTABLE_1, true);
    testCase.SetMask.setBit(RELAY_MONOSTABLE_1, true);     // switch MONOSTABLE_1 -> 1
    testCase.EnableMask.setBit(RELAY_MONOSTABLE_3, true);
    testCase.SetMask.setBit(RELAY_MONOSTABLE_3, true);     // switch MONOSTABLE_3 -> 1
    testCase.expectedLogicalCurrentMask.setBit(RELAY_MONOSTABLE_1, true);
    testCase.expectedLogicalCurrentMask.setBit(RELAY_MONOSTABLE_3, true);
    // init compare masks
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT);
    expectedData.clear();
    // switch 1
    resultEnableMask.setBit(PIN_MONOSTABLE_1, true);
    resultSetMask.setBit(PIN_MONOSTABLE_1, true);
    resultEnableMask.setBit(PIN_MONOSTABLE_3, true);
    resultSetMask.setBit(PIN_MONOSTABLE_3, true);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 0));
    // add testcase
    testCase.expectedData = expectedData;
    testCases.append(testCase);

    /* define test case */
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.Description = "Monostable1+3->0";
    testCase.bForce = false;
    testCase.EnableMask.setBit(RELAY_MONOSTABLE_1, true);
    testCase.SetMask.setBit(RELAY_MONOSTABLE_1, false);     // switch MONOSTABLE_1 -> 0
    testCase.EnableMask.setBit(RELAY_MONOSTABLE_3, true);
    testCase.SetMask.setBit(RELAY_MONOSTABLE_3, false);     // switch MONOSTABLE_3 -> 0
    testCase.expectedLogicalCurrentMask.setBit(RELAY_MONOSTABLE_1, false);
    testCase.expectedLogicalCurrentMask.setBit(RELAY_MONOSTABLE_3, false);
    // init compare masks
    resultEnableMask.fill(false, PHYSICAL_PIN_COUNT);
    resultSetMask.fill(false, PHYSICAL_PIN_COUNT);
    expectedData.clear();
    // switch 1
    resultEnableMask.setBit(PIN_MONOSTABLE_1, true);
    resultSetMask.setBit(PIN_MONOSTABLE_1, false);
    resultEnableMask.setBit(PIN_MONOSTABLE_3, true);
    resultSetMask.setBit(PIN_MONOSTABLE_3, false);
    expectedData.append(TExpectedLowLayerData(resultEnableMask, resultSetMask, 0));
    // add testcase
    testCase.expectedData = expectedData;
    testCases.append(testCase);

    /* define test case */
    testCase.SetMask.fill(false,LOGICAL_RELAY_COUNT);    // init
    testCase.EnableMask.fill(false,LOGICAL_RELAY_COUNT); // init
    testCase.Description = "No OP: Monostable1+3->0 Bistable 200ms->1 / 500ms->0";
    testCase.bForce = false;
    testCase.EnableMask.setBit(RELAY_MONOSTABLE_1, true);
    testCase.SetMask.setBit(RELAY_MONOSTABLE_1, false);     // switch MONOSTABLE_1 -> 0
    testCase.EnableMask.setBit(RELAY_MONOSTABLE_3, true);
    testCase.SetMask.setBit(RELAY_MONOSTABLE_3, false);     // switch MONOSTABLE_3 -> 0
    testCase.EnableMask.setBit(RELAY_BISTABLE_500_1, true);
    testCase.SetMask.setBit(RELAY_BISTABLE_500_1, false);     // switch RELAY_BISTABLE_500_1 -> 0
    testCase.EnableMask.setBit(RELAY_BISTABLE_200_1, true);
    testCase.SetMask.setBit(RELAY_BISTABLE_200_1, true);     // switch RELAY_BISTABLE_200_1 -> 1
    // we don't expect callbacks
    expectedData.clear();
    // add testcase
    testCase.expectedData = expectedData;
    testCases.append(testCase);

    /* setup relay mapper */
    bool bTestError = false;
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
        qint64 elapsed = timer.elapsed();
        timer.start();
        QString evaluationElapsed = "OK";
        QBitArray expectedEnableMask;
        QBitArray expectedSetMask;

        if(currTestCase<testCases.count())
        {
            if(currCallback < testCases[currTestCase].expectedData.size())
            {
                if(testCases[currTestCase].expectedData[currCallback].msSinceLast != 0)
                {
                    // Check delay deviation
                    double expectedDelay = testCases[currTestCase].expectedData[currCallback].msSinceLast;
                    double actualDelay = elapsed;
                    double deviation = (actualDelay-expectedDelay)/expectedDelay;
                    if(fabs(deviation) > 0.1 /* 10 % */)
                    {
                        bTestError = true;
                        evaluationElapsed = "Error: delay out of limit!!!";
                    }
                }
                // keep masks
                expectedEnableMask = testCases[currTestCase].expectedData[currCallback].EnableMask;
                expectedSetMask = testCases[currTestCase].expectedData[currCallback].SetMask;
            }
            else
            {
                bTestError = true;
                evaluationElapsed =
                        "Unconfigured response!!!";
            }

        }
        else
        {
            bTestError = true;
            evaluationElapsed = "Test case out of limit!!!";
        }

        qInfo() << "Callback: " << ++currCallback << "elapsed: " << elapsed << evaluationElapsed;
        if(expectedEnableMask == EnableMask)
            qInfo() << "EnableMask:" << EnableMask << "OK";
        else
        {
            bTestError = true;
            qInfo() << "EnableMask:" << EnableMask << "Error!!!" << expectedEnableMask;
        }
        if(expectedSetMask == SetMask)
            qInfo() << "SetMask:   " << SetMask << "OK";
        else
        {
            bTestError = true;
            qInfo() << "SetMask:   " << SetMask << "Error!!!" << expectedSetMask;
        }
        return false;
    }); // relay callback end


    // run all test cases in singleshot timer -> we need working evenloop
    QTimer::singleShot(300,[&]
                       ()
    {   // Timer callback begin
        int idleCount = 0;
        // Simple idle counter
        QObject::connect(
            &relayMapper, &QRelayMapper::idle,
            [&]() { idleCount++; }
        );
        for(;currTestCase<testCases.count(); currTestCase++)
        {
            currCallback = 0;
            bTestError = false;
            timer.start();
            QEventLoop loop;
            QObject::connect(&relayMapper, &QRelayMapper::idle, &loop, &QEventLoop::quit);
            if(currTestCase > 0)
                qInfo() << "";
            qInfo() << "Starting test case: " << testCases[currTestCase].Description;
            relayMapper.startSetMulti(
                        testCases[currTestCase].EnableMask,
                        testCases[currTestCase].SetMask,
                        testCases[currTestCase].bForce);
            loop.exec();

            // Check if all expected callbacks were called
            int expectedCallbacks = testCases[currTestCase].expectedData.size();
            if(currCallback < expectedCallbacks)
            {
                bTestError = true;
                qInfo() << "To few callback called Error!!!";
                qInfo() << "Expected: " << expectedCallbacks << "Received:" << currCallback;
            }
            // Check if the logical state is as expected
            QBitArray expectedLogicalMask = testCases[currTestCase].expectedLogicalCurrentMask;
            QBitArray actualLogicalMask = relayMapper.getLogicalRelayState();
            if(actualLogicalMask == expectedLogicalMask)
                qInfo() << "Actual mask logical: " << actualLogicalMask;
            else
            {
                bTestError = true;
                qInfo() << "Actual mask logical Error!!!";
                qInfo() << "Expected: " << expectedLogicalMask << "reported: " << actualLogicalMask;
            }
        }
        // correct number of idles received?
        if(idleCount==testCases.count())
            qInfo() << "Idle signals received" << idleCount;
        else
        {
            bTestError = true;
            qInfo() << "Idle signals received" << idleCount << "Expected: " << testCases.count();
        }
        // End message
        qInfo() << "";
        qInfo() << "";
        if(!bTestError)
        {
            qInfo() << "All tests passed.";
            a.exit(0);
        }
        else
        {
            qInfo() << "Error(s) occured!!!";
            a.exit(-1);
        }
    }); // Timer callback end

    return a.exec();
}
