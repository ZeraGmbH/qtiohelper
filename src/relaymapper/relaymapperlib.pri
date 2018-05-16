INCLUDEPATH += $$PWD

PUBLIC_HEADERS += \
    $$PWD/relaymapper_global.h \
    $$PWD/relay-base.h \
    $$PWD/relay-mapper.h \
    $$PWD/relay-sequencer.h \
    $$PWD/relay-serializer.h

PRIVATE_HEADERS += \
    $$PWD/relay-base_p.h \
    $$PWD/relay-mapper_p.h \
    $$PWD/relay-sequencer_p.h \
    $$PWD/relay-serializer_p.h

SOURCES += \
    $$PWD/relay-base.cpp \
    $$PWD/relay-mapper.cpp \
    $$PWD/relay-sequencer.cpp \
    $$PWD/relay-serializer.cpp

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
