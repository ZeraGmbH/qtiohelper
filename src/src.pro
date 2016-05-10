TEMPLATE = subdirs

SUBDIRS += \
    bit-input-poller \
    spidevice \
    tools \
    relay-mapper \
    remote-common

tools.depends = spidevice
spidevice.depends = remote-common
