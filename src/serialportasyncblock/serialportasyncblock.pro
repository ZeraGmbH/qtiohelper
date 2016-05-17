TARGET = QtSerialPortAsyncBlock
QT = core serialport

load(qt_module)

include($$PWD/serialportasyncblock-lib.pri)

PRECOMPILED_HEADER =

DEFINES += QT_SERIALPORTASYNCBLOCK_LIB
