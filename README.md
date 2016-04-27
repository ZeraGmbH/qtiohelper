QtIoHelper
============

## Overview

QtIoHelper is a set of tiny Qt I/O modules easing access on

* SPI: -> QT += spidevice

others might follow...

## Remote device access

Since 0.1.0 there is the possibility access SPI device nodes from remote
machines by simply:

* build tools/spi-remote-server and run on target supplying devices
```sh
spi-remote-server -p <port> 
```
* In the application under development call once
```sh
  QSPIDevice::setRemoteServer(<serverIP>, <serverPort>)
```

Happy remote device debugging :)
