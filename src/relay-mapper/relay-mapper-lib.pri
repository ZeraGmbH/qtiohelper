INCLUDEPATH += $$PWD

PUBLIC_HEADERS += \
    $$PWD/relay_global.h \
    $$PWD/relay-base.h \
    $$PWD/relay-mapper.h \
    $$PWD/relay-sequencer.h

PRIVATE_HEADERS += \
    $$PWD/relay-base_p.h \
    $$PWD/relay-mapper_p.h \
    $$PWD/relay-sequencer_p.h

SOURCES += \
    $$PWD/relay-base.cpp \
    $$PWD/relay-mapper.cpp \
    $$PWD/relay-sequencer.cpp

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
