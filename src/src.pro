TEMPLATE = subdirs

SUBDIRS += \
    actuasense \
    bit-input-poller \
    relay-mapper \
    remote-common \
    spidevice \
    serialportasyncblock \
    testapps \
    tools

testapps.depends = relay-mapper
tools.depends = spidevice
spidevice.depends = remote-common
