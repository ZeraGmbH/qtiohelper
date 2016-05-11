TEMPLATE = subdirs

SUBDIRS += \
    actuasense \
    bit-input-poller \
    relay-mapper \
    remote-common \
    spidevice \
    tools

tools.depends = spidevice
spidevice.depends = remote-common
