%modules = (
    "QtActuaSense"           => "$basedir/src/actuasense",
    "QtBitInputPoller"       => "$basedir/src/bit-input-poller",
    "QtIoRemoteCommon"       => "$basedir/src/remote-common",
    "QtRelayMapper"          => "$basedir/src/relay-mapper",
    "QtSerialPortAsyncBlock" => "$basedir/src/serialportasyncblock",
    "QtSpiDevice"            => "$basedir/src/spidevice",
);

%dependencies = (
        "qtbase"             => "",
);
