QT += core
QT -= gui
QT += network

CONFIG += c++11

TARGET = spi-remote-server
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

target.path = /usr/bin
INSTALLS += target

LIBS += -L../../../lib -lQt5SpiDevice -lQt5IoRemoteCommon
INCLUDEPATH +=  ../../../include ../../../include/QtSpiDevice
