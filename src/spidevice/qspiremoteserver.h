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
    QSPIDeviceRemoteServer(QObject *parent = 0);
    void open(quint16 port);
public slots:
    void onClientNew();
    void onClientDisconnect();

private:
    QTcpServer server;
    QHash<QTcpSocket*, QSPIDeviceServerClient*> clientHash;
};

#endif // QSPIREMOTESERVER_H
