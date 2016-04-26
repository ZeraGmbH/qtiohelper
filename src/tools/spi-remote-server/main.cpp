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
    if(optionsOK)
    {
        QSPIDeviceRemoteServer remoteServer;
        qInfo("SPI server listening on IPPort %i\n", port);
        remoteServer.open(port);
        return a.exec();
    }
    else
        return -1;
}
