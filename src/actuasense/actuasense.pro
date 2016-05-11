TARGET = QtActuaSense
QT = core

CONFIG += c++11

load(qt_module)

include($$PWD/actuasense-lib.pri)

PRECOMPILED_HEADER =
