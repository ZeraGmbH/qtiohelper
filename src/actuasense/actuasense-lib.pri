INCLUDEPATH += $$PWD

PUBLIC_HEADERS += \
    $$PWD/actuasense_global.h \
    $$PWD/actuasense.h

PRIVATE_HEADERS += \
    $$PWD/actuasense_p.h

SOURCES += \
    $$PWD/actuasense.cpp

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
