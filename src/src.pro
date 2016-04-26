TEMPLATE = subdirs

SUBDIRS += \
    spidevice \
    tools

tools.depends = spidevice
