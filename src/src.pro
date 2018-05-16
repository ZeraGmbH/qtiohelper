TEMPLATE = subdirs

SUBDIRS += \
    actuasense \
    bitinputpoller \
    relaymapper \
    remotecommon \
    spidevice \
    serialportasyncblock \
    testapps \
    tools

testapps.depends = relaymapper
tools.depends = spidevice
spidevice.depends = remotecommon
