TARGET = QtRelayMapper
QT = core

CONFIG += c++11

load(qt_module)

include($$PWD/relay-mapper-lib.pri)

PRECOMPILED_HEADER =

DEFINES += QT_BUILD_RELAYS_LIB
