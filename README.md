QtIoHelper
============

QtIoHelper is a set of tiny Qt I/O helper modules:

* **QT += actuasense (see acuasense.h):**

  Arrays of Actuators/Sensors (e.g pneumatics with valves and initiators) are setup and controlled. After initating actuator changes, actuator's sensors are monitored until target position is reached or timeout. After reaching target positions, the sensors are long term monitored (useful to detect e.g low preassure).

  Example setup:
  ```cpp
  #include <QActuaSense>
  
  class GpioInOut
  {
    public:
      void StartDigitalOut(const QBitArray& EnableMask, const QBitArray& SetMask);
  }
  
  ...
  GpioInOut gpioInOut;
  ...
  
  void SetupSPSIO(QActuaSense *sps, QBitInputPoller *bitInputPoller /* see below */)
  {
      /* Set cylinder controlled by one valves and checked by two sensors */
      sps->addAtomicOut(SPS_IO_PUSHBUTTON,          24);
      sps->addAtomicIn (SPS_IO_PUSHBUTTON_PRESSED,   0);
      sps->addAtomicIn (SPS_IO_PUSHBUTTON_RELEASED,  1);
      
      /* Set cylinder controlled by two valves and checked by two sensors */
      sps->addAtomicOut(SPS_IO_NOT_CONTACT,         25);
      sps->addAtomicIn (SPS_IO_NOT_CONTACT,          2);
      sps->addAtomicOut(SPS_IO_CONTACT,             26);
      sps->addAtomicIn (SPS_IO_CONTACT,              3);

      /* Set total bitmask size (optional) */
      sps->setBitMaskSize(SPS_OUT_BIT_COUNT);
      /* Get our sensor in bitmask from input poller */
      sps.setInBitMask(bitInputPoller->getInputBitmask());
      /* Reuse timer from input poller / or alternate: startInternalInputPoll(int milliSeconds); */
      QObject::connect(
        bitInputPoller, &QBitInputPoller::bitmaskUpdated, sps, &QActuaSense::onPollTimer);
      /* Callback transferring bits to outputs (in love with lambda callbacks :) */
      sps.setStartLowLayerCallback( [&]
                                    (
                                      const QBitArray& EnableMask,
                                      const QBitArray& SetMask
                                    )

                {
                    /* A finished action is indirectly detected by sensors.
                    So we don't care for blocked/unblocked here */
                    gpioInOut.StartDigitalOut(EnableMask, SetMask);
                });
  }
  ```
  
  Example action:
  ```cpp
  void StartPushButton(QActuaSense *sps, bool bPush)
  {
    qInfo(bPush ? "Pushing button..." : "Releasing button...");
    sps->openMultiAction();
    /* set output */
    sps->startOutSet(SPS_IO_PUSHBUTTON, bPush);
    /* setup desired input and messages */
    sps->startInObserve(SPS_IO_PUSHBUTTON_RELEASED, !bPush, 2000 /* timeoutMs */,
                         !bPush ? QLatin1String("Pushbutton released position was reached successful") : QString(),
                         !bPush ? QLatin1String("Pushbutton released position was not reached!") : QString(),
                         !bPush ? QLatin1String("Pushbutton released position got lost!") : QString());
    sps->startInObserve(SPS_IO_PUSHBUTTON_PRESSED, bPush, 2000,
                          bPush ? QLatin1String("Pushbutton pressed position was reached successful") : QString(),
                          bPush ? QLatin1String("Pushbutton pressed position was not reached!") : QString(),
                          bPush ? QLatin1String("Pushbutton pressed position got lost!") : QString());
    sps->closeMultiAction();
  }
  ```
  
  End of action (success/timeout) is notified by signal:
  ```cpp
    void multiActionFinished(bool bError);
  ```
  
  Loosing end position (e.g low pressure) is notified by signal:
  ```cpp
    void longTermObservationError();
  ```

---

* **QT += bit-input-poller (see bit-input-poller.h):**

  A very simple module to poll an array of input bits periodically.

  Example setup:
  ```cpp
  #include <QBitInputPoller>
  
  class GpioInOut
  {
    public:
      void GetDigitalInputs(QBitArray *inputMask);
  }
  
  ...
  GpioInOut gpioInOut;
  ...
  QBitInputPoller bitInputPoller;
  QBitArray invertMask(4, true); /* invert bit 0-3 */
  bitInputPoller.setupInputMask(SPS_IN_BIT_COUNT, &invertMask);
  bitInputPoller.setStartBitReadFunction([&] (QBitArray *pDigitalInMask)
      {
        gpioInOut.GetDigitalInputs(pDigitalInMask);
        /* blocking */
        return false;
      });
  ```
  
  At every time input state can be checked by
  ```cpp
  QBitArray *array = bitInputPoller.getInputBitmask();
  ```
  
  After every poll the signal
  ```cpp
  void bitmaskUpdated();
  ```
  if fired.

---

* **QT += relay-mapper (see relay-mapper.h):**

  QRelayMapper handles arrays of logical relays as objects of QBitArray. A logical relay is the uniform representation of a physical relay and abstracts away:
  * mono-/bistable relays: Monostable (one coil / loosing 'on' state on power off) and bistable (two coils / switch on only during state change / keeping current state on power off) relays are switched in the same way (see startSet implementations). For bistable relays QRelayMapper takes care for correct timing of coils.
  * all relays are switched on by value 'true' ond off by value 'false'
  * when all transitions are finished QRelayMapper **fires signal idle()**.
  * current relay state is kept: In case a client calls 'startSet' with the same state as currently set, no action is performed and in case not further actions are pending an idle() signal is fired.
  
  Example setup:
  ```cpp
  #include <QRelayMapper>
  
  class GpioInOut
  {
    public:
      void StartDigitalOut(const QBitArray& EnableMask, const QBitArray& SetMask);
  }
  
  ...
  GpioInOut gpioInOut;
  ...
  
  enum
  {
    LOGICAL_RELAY_BISTABLE_FOO = 0,  
    LOGICAL_RELAY_MONOSTABLE_FOO,
    
    LOGICAL_RELAY_COUNT
  };
  
  const struct TLogicalRelayEntry arrRelayMapperSetup[LOGICAL_RELAY_COUNT] =
  {
    { /* LOGICAL_RELAY_BISTABLE_FOO */
      .ui16OnPosition  = 0, /* on coil is connected to bit 0 of output array */
      .ui16OffPosition = 1, /* off coil is connected to bit 1 of output array */
      .ui8Flags = RELAY_PHYS_FLAGS(true, false, false),  /* bistable no inverting pins */
      .ui8OnTime = 2, // 20ms @ 10ms tick
    },
  
    { /* LOGICAL_RELAY_MONOSTABLE_FOO */
      .ui16OnPosition  = 2, /* on coil is connected to bit 2 of output array */
      .ui8Flags = RELAY_PHYS_FLAGS(false, true, false),  /* monotable no inverting pin */
    }
  };

  QRelayMapper relayMapper;
  relayMapper.setup(LOGICAL_RELAY_COUNT,
                    arrRelayMapperSetup,
                    10,   // 10ms
                    [&]
                    (
                      const QBitArray& EnableMask,
                      const QBitArray& SetMask,
                      const QObject* pCaller      /* pointer to relayMapper */
                    )

              {
                  gpioInOut.StartDigitalOut(EnableMask, SetMask);
                  return false;
              });
  ```

  Example Action:
  ```cpp
  relayMapper.startSet(LOGICAL_RELAY_BISTABLE_FOO, true);
  relayMapper.startSet(LOGICAL_RELAY_MONOSTABLE_FOO, false);
  ```
  
---

* **QT += serialportasyncblock (see serialportasyncblock.h):**

  QSerialPortAsyncBlock extends QSerialPort by detection of end condition when sending and receiving serial data. An end condition can be a timeout, a pattern in data received or both.
  
  Example:
  ```cpp
  #include <QSerialPortAsyncBlock>
  
  /* RS232 open */
  QSerialPortAsyncBlock serialIO;
  serialIO.setPortName("ttyUSB0");
  serialIO.open(QIODevice::ReadWrite);

  /* 9600 Baud / 7E1 / 1 StopBit */
  serialIO.setBaudRate(9600);
  serialIO.setDataBits(QSerialPort::Data7);
  serialIO.setParity(QSerialPort::EvenParity);
  serialIO.setStopBits(QSerialPort::OneStop);

  /* set up block end detection conditions */
  QByteArray dataEnd("\r\n");
  int iResponseTimeout = 2000; /* 2000ms offset till first byte received */
  int iTimeoutBetweenBytes = 100; /* 100ms maximum time between two received bytes */
  serialIO.setReadTimeout(iResponseTimeout, iTimeoutBetweenBytes);
  serialIO.setBlockEndCriteria(0, dataEnd); /* iBlockLenReceive=0: we have varying response lengths */

  /* start I/O */
  QByteArray dataSend("/?!\r\n");
  m_pSerialPort->sendAndReceive(dataSend, &m_ReceivedRaw);
  ```
  End of send and receive is notified by firing signal:
  ```cpp
  void ioFinished();
  ```
  Just receiving data without sending can be started by calling sendAndReceive with empty buffer:
  ```cpp
  serialIO.sendAndReceive(QByteArray(), &m_ReceivedRaw);
  ```

---
  
* **QT += spidevice (see spidevice.h):**

  A Qt Wrapper controlling SPI devices. All unidirectional I/O is performed by blocking (kernel currenly only supports blocking accesses but SPI transactions are usually fast) read/write calls. For Bidirectional I/O use the function
  
  ```cpp
  #include <QSPIDevice>
  
  bool sendReceive(QByteArray& dataSend, QByteArray& dataReceive);
  ```
  
  Example setup:
  
  ```cpp
  QSPIDevice spiDev(1, 0); /* use bus=1 / channel=0 */
  QString strOpenErrorSpiDev;
  // SPI open data
  if(spiDev.open(QIODevice::ReadWrite))
  {
    /* for more information see kernel linux/spi/spidev.h or Documentation/spi/spidev */
    spiDev.setBitSpeed(25000000);
    spiDev.setMode(3);
    spiDev.setLSBFirst(false);
    spiDev.setBitsPerWord(8);
  }
  else
  {
    strOpenErrorSpiDev.sprintf("Device %s could not be opened!", qPrintable(spiDev.fileName()));
    qWarning(qPrintable(strOpenErrorSpiDev));
  }
  ```
  **One very very important feature** is the remote access e.g to develop/debug an appliaction on PC using SPI on a development board connected by network without cross building necessary until packing. For remote SPI access do:
  
  * Start
  ```sh
  spi-remote-server -p <IP-port> --v <verbosity-level [0..3]>
  ```
    on development board
  
  * Add to code:
  ```cpp
  QSPIDevice::setRemoteServer(<IP>, <port>);
  ```
    compile and run it on PC. Note that
    
    * all SPI accesses are **fully transparent** - it feels as if the SPIs are on the PC. Device names are those found on development board.
    * a high **verbosity level is very helpful for debugging**: the bytes transfered are displayed (but also slows down communication).

