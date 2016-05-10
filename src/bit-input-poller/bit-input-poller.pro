TARGET = QtBitInputPoller
QT = core

CONFIG += c++11

load(qt_module)

include($$PWD/bit-input-poller-lib.pri)

PRECOMPILED_HEADER =

DEFINES += QT_BUILD_BITINPUTPOLLER_LIB
