TEMPLATE = subdirs

SUBDIRS += \
    spidevice \
    tools \
    relay-mapper \
    remote-common

tools.depends = spidevice
spidevice.depends = remote-common
