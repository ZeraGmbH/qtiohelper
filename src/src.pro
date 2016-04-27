TEMPLATE = subdirs

SUBDIRS += \
    spidevice \
    tools \
    remote-common

tools.depends = spidevice
spidevice.depends = remote-common
