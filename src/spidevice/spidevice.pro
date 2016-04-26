TARGET = QtSpiDevice
QT = core
QT += network

load(qt_module)

include($$PWD/spidevice-lib.pri)

PRECOMPILED_HEADER =
