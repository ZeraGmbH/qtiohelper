INCLUDEPATH += $$PWD

PUBLIC_HEADERS += \
    $$PWD/bit-input-poller_global.h \
    $$PWD/bit-input-poller.h

PRIVATE_HEADERS += \
    $$PWD/bit-input-poller_p.h

SOURCES += \
    $$PWD/bit-input-poller.cpp

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
