#include <QCoreApplication>
#include <QCommandLineParser>
#include <QtSpiDevice>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("SPI remote server");
    parser.addHelpOption();

    // option for port number
    QCommandLineOption portOption(QStringList() << "p" << "port", "IP-port number", "portnumber");
    portOption.setDefaultValue("5000");
    parser.addOption(portOption);

    // option for port verbosity
    QCommandLineOption verboseOption(QStringList() << "v" << "verbose", "Debug output level [0..3]", "level");
    verboseOption.setDefaultValue("1");
    parser.addOption(verboseOption);

    parser.process(a);
    bool optValOK = true;
    bool optionsOK = true;

    QString strOptVal = parser.value(portOption);
    int port = strOptVal.toInt(&optValOK);
    if(port < 0 || port > 65535)
        optValOK = false;
    if(!optValOK)
    {
        qWarning("Invalid value for IPPort %s!\n", qPrintable(strOptVal));
        optionsOK = false;
    }

    strOptVal = parser.value(verboseOption);
    int verboseLevel = strOptVal.toInt(&optValOK);
    if(!optValOK || verboseLevel<0 || verboseLevel>3)
    {
        qWarning("Invalid value for verbose level %s!\n", qPrintable(strOptVal));
        optionsOK = false;
    }

    if(optionsOK)
    {
        QSPIDeviceRemoteServer remoteServer;
        remoteServer.setVerboseLevel(verboseLevel);
        remoteServer.open(port);
        return a.exec();
    }
    else
        return -1;
}
