INCLUDEPATH += $$PWD

PUBLIC_HEADERS += \
    $$PWD/relay-mapper_global.h \
    $$PWD/relay-mapper.h

PRIVATE_HEADERS += \
    $$PWD/relay-mapper_p.h

SOURCES += \
    $$PWD/relay-mapper.cpp

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
