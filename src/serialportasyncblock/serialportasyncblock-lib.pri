INCLUDEPATH += $$PWD

PUBLIC_HEADERS += \
    $$PWD/serialportasyncblock_global.h \
    $$PWD/serialportasyncblock.h

PRIVATE_HEADERS += \
    $$PWD/serialportasyncblock_p.h

SOURCES += \
    $$PWD/serialportasyncblock.cpp

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

#QMAKE_CXXFLAGS += -O0
