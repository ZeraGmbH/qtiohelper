%modules = (
    "QtActuaSense"           => "$basedir/src/actuasense",
    "QtBitInputPoller"       => "$basedir/src/bitinputpoller",
    "QtIoRemoteCommon"       => "$basedir/src/remotecommon",
    "QtRelayMapper"          => "$basedir/src/relaymapper",
    "QtSerialPortAsyncBlock" => "$basedir/src/serialportasyncblock",
    "QtSpiDevice"            => "$basedir/src/spidevice",
);

%dependencies = (
        "qtbase"             => "",
);
