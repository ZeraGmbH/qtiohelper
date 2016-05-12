#ifndef QSPIDEVICE_P_H
#define QSPIDEVICE_P_H

#include <QObject>
#include <QFile>
#include <QTcpSocket>
#include <QTcpServer>
#include "qspidevice.h"

class QSPIDeviceRemoteClient;

class QSPIDevicePrivate
{
public:
    QSPIDevicePrivate();
    virtual ~QSPIDevicePrivate();
    /**
      At least beaglebone's SPI does not support LSBFirst. We implement a
      software fallback with the assumption that MSBFirst is default. So
      when LSBFirst cannot be set, bSWReverseRequired is set and the
      read/write buffers are reversed.
      */
    bool bSWReverseRequired;
    QSPIDeviceRemoteClient *remoteClient;
    QString devFileName;
};

class QSPIDeviceRemoteClient
{
public:
    bool open(const QString serverIP, quint16 serverPort, QString devFileName, QIODevice::OpenMode flags);
    void close();
    bool setMode(quint8 ui8Mode);
    bool setLSBFirst(bool lsbFirst);
    bool setBitsPerWord(quint8 ui8BitsPerWord);
    bool setBitSpeed(quint32 ui32BitSpeedHz);
    bool sendReceive(QByteArray& dataRemoteSend, QByteArray& dataRemoteReceive);
    qint64 readData(char *data, qint64 i64maxlen);
    qint64 writeData(const char *data, qint64 i64Len);
private:
    QTcpSocket socket;
};

class QSPIDeviceServerClient: public QObject
{
    Q_OBJECT
public:
    QSPIDeviceServerClient(QObject *parent, QTcpSocket* sock);
    void setVerboseLevel(int level);

public slots:
    void onReceive();

private:
    void handleOpen(QByteArray *dataReceive);
    void handleClose(QByteArray *dataReceive);
    void handleSetMode(QByteArray* dataReceive);
    void handleSetLSBFirst(QByteArray* dataReceive);
    void handleSetBitsPerWord(QByteArray* dataReceive);
    void handleSetBitSpeed(QByteArray* dataReceive);
    void handleSendReceive(QByteArray* dataReceive);
    void handleReadData(QByteArray* dataReceive);
    void handleWriteData(QByteArray* dataReceive);

    void logData(QString strLead, QByteArray data);

    QTcpSocket* socket;
    QSPIDevice spiDevice;
    int verboseLevel;
};

#endif // QSPIDEVICE_P_H

