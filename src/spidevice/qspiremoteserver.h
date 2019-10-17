#ifndef QSPIREMOTESERVER_H
#define QSPIREMOTESERVER_H

#include <QTcpSocket>
#include <QTcpServer>
#include "qspidevice.h"

class QSPIDeviceServerClient;

class QTSPIDEVICESHARED_EXPORT QSPIDeviceRemoteServer: public QObject
{
    Q_OBJECT
public:
    QSPIDeviceRemoteServer(QObject *parent = nullptr);
    void open(quint16 port);
    void setVerboseLevel(int level);
public slots:
    void onClientNew();
    void onClientDisconnect();

private:
    QTcpServer server;
    QHash<QTcpSocket*, QSPIDeviceServerClient*> clientHash;
    int verboseLevel;
};

#endif // QSPIREMOTESERVER_H
