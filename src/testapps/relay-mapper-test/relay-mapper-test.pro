QT += core
QT -= gui

CONFIG += c++11

TARGET = relay-mapper-test
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

target.path = /usr/bin
INSTALLS += target

LIBS += -L../../../lib -lQt5RelayMapper
INCLUDEPATH +=  ../../../include ../../../include/QtRelayMapper
