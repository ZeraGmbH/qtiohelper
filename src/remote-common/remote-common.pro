TARGET = QtIoRemoteCommon
QT = core
QT += network

load(qt_module)

include($$PWD/remote-common-lib.pri)

DEFINES += QIOHELPERREMOTE_LIBRARY
