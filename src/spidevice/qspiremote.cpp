#include <QDataStream>
#include <QEventLoop>
#include <QTimer>
#include <limits>
#include "qspidevice_p.h"
#include "qspiremoteserver.h"

/////////////////////////////////////////////////////////////////////////////////////////////
// TODO Have a more common place for these
bool readTCPFrameBlocked(QTcpSocket *socket, QByteArray *data)
{
    QTimer timer;
    timer.setSingleShot(true);

    QEventLoop loop;
    loop.connect(socket, SIGNAL(readyRead()), SLOT(quit()));
    loop.connect(&timer, SIGNAL(timeout()), SLOT(quit()));
    timer.start(2000);

    data->clear();
    // we have to read leading length first
    quint32 ui32Count = sizeof(ui32Count);
    QDataStream stream(data, QIODevice::ReadOnly);
    bool bLenValid = false;
    while ((quint32)data->size() < ui32Count)
    {
        // do wait only in case in data isn't already available
        if(socket->bytesAvailable() == 0)
            loop.exec();
        // readyRead?
        if(timer.isActive())
        {
            *data += socket->read(qMin(ui32Count, (quint32)socket->bytesAvailable()));
            if(!bLenValid && (quint32)data->size() >= sizeof(ui32Count))
            {
                stream >> ui32Count;
                bLenValid = true;
                // we just read count there might be more waiting
                *data += socket->read(qMin(ui32Count, (quint32)socket->bytesAvailable()));
            }
        }
        // timeout
        else
            break;
    }
    return ui32Count == (quint32)data->size();
}

void sendOK(QTcpSocket *socket, bool bOK)
{
    quint32 ui32Len = 0;
    quint8 ui8OK = bOK;

    // create response
    QByteArray dataCmdSend;
    QDataStream streamOut(&dataCmdSend, QIODevice::WriteOnly);
    streamOut << ui32Len << ui8OK;

    // length correction
    ui32Len = dataCmdSend.size();
    streamOut.device()->seek(0);
    streamOut << ui32Len;

    // send response
    socket->write(dataCmdSend);
}

bool receiveOK(QTcpSocket *socket)
{
    // get response
    QByteArray dataReceive;
    bool bOK = false;
    if(readTCPFrameBlocked(socket, &dataReceive))
    {
        quint32 ui32Len;
        quint8 ui8OK;
        QDataStream streamIn(&dataReceive, QIODevice::ReadOnly);
        streamIn >> ui32Len >> ui8OK;
        bOK = ui8OK;
    }
    return bOK;
}

/////////////////////////////////////////////////////////////////////////////////////////////
enum enRemotableCommands
{
    CMD_OPEN = 0,
    CMD_CLOSE,
    CMD_SETMODE,
    CMD_LSBFIRST,
    CMD_BITSPERWORD,
    CMD_BITSPEED,
    CMD_SENDRECEIVE,
    CMD_READ,
    CMD_WRITE
};

bool QSPIDeviceRemoteClient::open(const QString serverIP, quint16 serverPort, QString devFileName, QIODevice::OpenMode flags)
{
    QTimer timer;
    timer.setSingleShot(true);

    QEventLoop loop;
    QObject::connect(&socket, &QTcpSocket::connected, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(2000);

    // connect and wait for establishment
    socket.connectToHost(serverIP, serverPort);
    loop.exec();

    QByteArray dataCmdSend;
    QDataStream streamOut(&dataCmdSend, QIODevice::WriteOnly);

    // Build send frame
    quint32 ui32Len = 0;
    quint32 ui32Cmd = CMD_OPEN;
    quint32 ui32flags = flags;
    streamOut << ui32Len << ui32Cmd << devFileName << ui32flags;

    // length correction
    ui32Len = dataCmdSend.size();
    streamOut.device()->seek(0);
    streamOut << ui32Len;

    // send
    socket.write(dataCmdSend);

    // get response
    return receiveOK(&socket);
}

void QSPIDeviceServerClient::handleOpen(QByteArray* dataReceive)
{
    // Extract cmd/data
    QDataStream streamIn(dataReceive, QIODevice::ReadOnly);
    quint32 ui32Len, ui32Cmd, ui32flags;
    QString devFileName;
    streamIn >> ui32Len >> ui32Cmd >> devFileName >> ui32flags;

    // handle device action
    spiDevice.setFileName(devFileName);
    bool bOK = spiDevice.open((QIODevice::OpenMode)ui32flags);

    // send response
    sendOK(socket, bOK);
}

void QSPIDeviceRemoteClient::close()
{
    QByteArray dataCmdSend;
    QDataStream streamOut(&dataCmdSend, QIODevice::WriteOnly);

    // Build send frame
    quint32 ui32Len = 0;
    quint32 ui32Cmd = CMD_CLOSE;
    streamOut << ui32Len << ui32Cmd;

    // length correction
    ui32Len = dataCmdSend.size();
    streamOut.device()->seek(0);
    streamOut << ui32Len;

    // send
    socket.write(dataCmdSend);

    // receive response without data content
    QByteArray dataReceive;
    readTCPFrameBlocked(&socket, &dataReceive);
}

void QSPIDeviceServerClient::handleClose(QByteArray* dataReceive)
{
    // Extract cmd/data
    QDataStream stream(dataReceive, QIODevice::ReadOnly);
    quint32 ui32Len, ui32Cmd;
    stream >> ui32Len >> ui32Cmd;

    // handle device action
    spiDevice.close();

    // create response
    QByteArray dataCmdSend;
    QDataStream streamOut(&dataCmdSend, QIODevice::WriteOnly);
    streamOut << ui32Len;

    // length correction
    ui32Len = dataCmdSend.size();
    streamOut.device()->seek(0);
    streamOut << ui32Len;

    // send response
    socket->write(dataCmdSend);
}

bool QSPIDeviceRemoteClient::setMode(quint8 ui8Mode)
{
    QByteArray dataCmdSend;
    QDataStream streamOut(&dataCmdSend, QIODevice::WriteOnly);

    // Build send frame
    quint32 ui32Len = 0;
    quint32 ui32Cmd = CMD_SETMODE;
    streamOut << ui32Len << ui32Cmd << ui8Mode;

    // length correction
    ui32Len = dataCmdSend.size();
    streamOut.device()->seek(0);
    streamOut << ui32Len;

    // send
    socket.write(dataCmdSend);

    // get response
    return receiveOK(&socket);
}

void QSPIDeviceServerClient::handleSetMode(QByteArray* dataReceive)
{
    // Extract cmd/data
    QDataStream streamIn(dataReceive, QIODevice::ReadOnly);
    quint32 ui32Len, ui32Cmd;
    quint8 ui8Mode;
    streamIn >> ui32Len >> ui32Cmd >> ui8Mode;

    // handle device action
    bool bOK = spiDevice.setMode(ui8Mode);

    // send response
    sendOK(socket, bOK);
}

bool QSPIDeviceRemoteClient::setLSBFirst(bool lsbFirst)
{
    QByteArray dataCmdSend;
    QDataStream streamOut(&dataCmdSend, QIODevice::WriteOnly);

    // Build send frame
    quint32 ui32Len = 0;
    quint32 ui32Cmd = CMD_LSBFIRST;
    quint8 ui8LsbFirst = lsbFirst;
    streamOut << ui32Len << ui32Cmd << ui8LsbFirst;

    // length correction
    ui32Len = dataCmdSend.size();
    streamOut.device()->seek(0);
    streamOut << ui32Len;

    // send
    socket.write(dataCmdSend);

    // get response
    return receiveOK(&socket);
}

void QSPIDeviceServerClient::handleSetLSBFirst(QByteArray* dataReceive)
{
    // Extract cmd/data
    QDataStream streamIn(dataReceive, QIODevice::ReadOnly);
    quint32 ui32Len, ui32Cmd;
    quint8 ui8LsbFirst;
    streamIn >> ui32Len >> ui32Cmd >> ui8LsbFirst;

    // handle device action
    bool bOK = spiDevice.setLSBFirst(ui8LsbFirst);

    // send response
    sendOK(socket, bOK);
}

bool QSPIDeviceRemoteClient::setBitsPerWord(quint8 ui8BitsPerWord)
{
    QByteArray dataCmdSend;
    QDataStream streamOut(&dataCmdSend, QIODevice::WriteOnly);

    // Build send frame
    quint32 ui32Len = 0;
    quint32 ui32Cmd = CMD_BITSPERWORD;
    streamOut << ui32Len << ui32Cmd << ui8BitsPerWord;

    // length correction
    ui32Len = dataCmdSend.size();
    streamOut.device()->seek(0);
    streamOut << ui32Len;

    // send
    socket.write(dataCmdSend);

    // get response
    return receiveOK(&socket);
}

void QSPIDeviceServerClient::handleSetBitsPerWord(QByteArray* dataReceive)
{
    // Extract cmd/data
    QDataStream streamIn(dataReceive, QIODevice::ReadOnly);
    quint32 ui32Len, ui32Cmd;
    quint8 ui8BitsPerWord;
    streamIn >> ui32Len >> ui32Cmd >> ui8BitsPerWord;

    // handle device action
    bool bOK = spiDevice.setBitsPerWord(ui8BitsPerWord);

    // send response
    sendOK(socket, bOK);
}

bool QSPIDeviceRemoteClient::setBitSpeed(quint32 ui32BitSpeedHz)
{
    QByteArray dataCmdSend;
    QDataStream streamOut(&dataCmdSend, QIODevice::WriteOnly);

    // Build send frame
    quint32 ui32Len = 0;
    quint32 ui32Cmd = CMD_BITSPEED;
    streamOut << ui32Len << ui32Cmd << ui32BitSpeedHz;

    // length correction
    ui32Len = dataCmdSend.size();
    streamOut.device()->seek(0);
    streamOut << ui32Len;

    // send
    socket.write(dataCmdSend);

    // get response
    return receiveOK(&socket);
}

void QSPIDeviceServerClient::handleSetBitSpeed(QByteArray* dataReceive)
{
    // Extract cmd/data
    QDataStream streamIn(dataReceive, QIODevice::ReadOnly);
    quint32 ui32Len, ui32Cmd;
    quint32 ui32BitSpeedHz;
    streamIn >> ui32Len >> ui32Cmd >> ui32BitSpeedHz;

    // handle device action
    bool bOK = spiDevice.setBitSpeed(ui32BitSpeedHz);

    // send response
    sendOK(socket, bOK);
}

bool QSPIDeviceRemoteClient::sendReceive(QByteArray& dataRemoteSend, QByteArray& dataRemoteReceive)
{
    QByteArray dataCmdSend;
    QDataStream streamOut(&dataCmdSend, QIODevice::WriteOnly);

    // Build send frame
    quint32 ui32Len = 0;
    quint32 ui32Cmd = CMD_SENDRECEIVE;
    streamOut << ui32Len << ui32Cmd << dataRemoteSend;

    // length correction
    ui32Len = dataCmdSend.size();
    streamOut.device()->seek(0);
    streamOut << ui32Len;

    // send
    socket.write(dataCmdSend);

    // get response
    bool bOK = false;
    QByteArray dataReceive;
    if(readTCPFrameBlocked(&socket, &dataReceive))
    {
        dataRemoteReceive.clear();
        quint8 ui8OK;
        QDataStream streamIn(dataReceive);
        streamIn >> ui32Len >> dataRemoteReceive >> ui8OK;
        bOK = ui8OK;
    }
    return bOK;
}

void QSPIDeviceServerClient::handleSendReceive(QByteArray* dataReceive)
{
    // Extract cmd/data
    QDataStream streamIn(dataReceive, QIODevice::ReadOnly);
    quint32 ui32Len, ui32Cmd;
    QByteArray dataRemoteSend, dataRemoteReceive;
    streamIn >> ui32Len >> ui32Cmd >> dataRemoteSend;

    // handle device action
    quint8 ui8OK = spiDevice.sendReceive(dataRemoteSend, dataRemoteReceive);

    // create response
    QByteArray dataCmdSend;
    QDataStream streamOut(&dataCmdSend, QIODevice::WriteOnly);
    streamOut << ui32Len << dataRemoteReceive << ui8OK;

    // length correction
    ui32Len = dataCmdSend.size();
    streamOut.device()->seek(0);
    streamOut << ui32Len;

    // send response
    socket->write(dataCmdSend);
}

qint64 QSPIDeviceRemoteClient::readData(char *data, qint64 ui64maxlen)
{
    QByteArray dataCmdSend;
    QDataStream streamOut(&dataCmdSend, QIODevice::WriteOnly);

    // Build send frame
    quint32 ui32Len = 0;
    quint32 ui32Cmd = CMD_READ;
    streamOut << ui32Len << ui32Cmd << ui64maxlen;

    // length correction
    ui32Len = dataCmdSend.size();
    streamOut.device()->seek(0);
    streamOut << ui32Len;

    // send
    socket.write(dataCmdSend);

    // get response
    QByteArray dataReceive;
    QByteArray dataRemoteReceive;
    if(readTCPFrameBlocked(&socket, &dataReceive))
    {
        QDataStream streamIn(dataReceive);
        streamIn >> ui32Len >> dataRemoteReceive;
        for(int iByte=0; iByte<dataRemoteReceive.size(); iByte++)
            data[iByte] = dataRemoteReceive[iByte];
    }
    return dataRemoteReceive.size();
}

void QSPIDeviceServerClient::handleReadData(QByteArray* dataReceive)
{
    // Extract cmd/data
    QDataStream streamIn(dataReceive, QIODevice::ReadOnly);
    quint32 ui32Len, ui32Cmd;
    qint64 ui64maxlen;
    streamIn >> ui32Len >> ui32Cmd >> ui64maxlen;

    // handle device action
    QByteArray dataRemoteReceive = spiDevice.read(ui64maxlen);

    // create response
    QByteArray dataCmdSend;
    QDataStream streamOut(&dataCmdSend, QIODevice::WriteOnly);
    streamOut << ui32Len << dataRemoteReceive;

    // length correction
    ui32Len = dataCmdSend.size();
    streamOut.device()->seek(0);
    streamOut << ui32Len;

    // send response
    socket->write(dataCmdSend);
}

qint64 QSPIDeviceRemoteClient::writeData(const char *data, qint64 i64Len)
{
    QByteArray dataCmdSend;
    QDataStream streamOut(&dataCmdSend, QIODevice::WriteOnly);

    // Build send frame
    quint32 ui32Len = 0;
    quint32 ui32Cmd = CMD_WRITE;
    QByteArray dataRemoteWrite = QByteArray::fromRawData(data, i64Len);
    streamOut << ui32Len << ui32Cmd << dataRemoteWrite;

    // length correction
    ui32Len = dataCmdSend.size();
    streamOut.device()->seek(0);
    streamOut << ui32Len;

    // send
    socket.write(dataCmdSend);

    // get response
    QByteArray dataReceive;
    qint64 ui64BytesWritten = 0;
    if(readTCPFrameBlocked(&socket, &dataReceive))
    {
        QDataStream streamIn(&dataReceive, QIODevice::ReadOnly);
        streamIn >> ui32Len >> ui64BytesWritten;
    }
    return ui64BytesWritten;
}

void QSPIDeviceServerClient::handleWriteData(QByteArray* dataReceive)
{
    // Extract cmd/data
    QDataStream streamIn(dataReceive, QIODevice::ReadOnly);
    quint32 ui32Len, ui32Cmd;
    QByteArray dataRemoteWrite;
    streamIn >> ui32Len >> ui32Cmd >> dataRemoteWrite;

    // handle device action
    qint64 i64BytesWritten = spiDevice.write(dataRemoteWrite);

    // create response
    QByteArray dataCmdSend;
    QDataStream streamOut(&dataCmdSend, QIODevice::WriteOnly);
    streamOut << ui32Len << i64BytesWritten;

    // length correction
    ui32Len = dataCmdSend.size();
    streamOut.device()->seek(0);
    streamOut << ui32Len;

    // send response
    socket->write(dataCmdSend);
}


/////////////////////////////////////////////////////////////////////////////////////////////
QSPIDeviceRemoteServer::QSPIDeviceRemoteServer(QObject* parent) : QObject(parent)
{
    connect(&server, &QTcpServer::newConnection, this, &QSPIDeviceRemoteServer::onClientNew);
}

void QSPIDeviceRemoteServer::open(quint16 port)
{
    server.listen(QHostAddress::Any, port);
}


void QSPIDeviceRemoteServer::onClientNew()
{
    QTcpSocket* pClientSocket = server.nextPendingConnection();
    qInfo("SPI client connected from %s\n", qPrintable(pClientSocket->peerAddress().toString()));
    QSPIDeviceServerClient* pClient = new QSPIDeviceServerClient(this, pClientSocket);
    clientHash[pClientSocket] = pClient;

    connect(pClientSocket, &QTcpSocket::disconnected, this, &QSPIDeviceRemoteServer::onClientDisconnect);
}

void QSPIDeviceRemoteServer::onClientDisconnect()
{
    QTcpSocket *pSocket = qobject_cast<QTcpSocket*>(sender());
    qInfo("SPI client %s disconnected\n", qPrintable(pSocket->peerAddress().toString()));
    if(pSocket)
    {
        QSPIDeviceServerClient* pClient = clientHash.take(pSocket);
        delete pClient;
        pSocket->deleteLater();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
QSPIDeviceServerClient::QSPIDeviceServerClient(QObject *parent, QTcpSocket* sock) : QObject(parent), spiDevice(0,0)
{
    socket = sock;
    connect(socket, &QTcpSocket::readyRead, this, &QSPIDeviceServerClient::onReceive);
}

void QSPIDeviceServerClient::onReceive()
{
    QTcpSocket *sendSocket = qobject_cast<QTcpSocket*>(sender());
    if(sendSocket == socket)
    {
        QByteArray dataReceive;
        if(readTCPFrameBlocked(socket, &dataReceive))
        {
            QDataStream stream(dataReceive);
            quint32 ui32Len, ui32Cmd;
            stream >> ui32Len >> ui32Cmd;
            switch(ui32Cmd)
            {
            case CMD_OPEN:
                handleOpen(&dataReceive);
                break;
            case CMD_CLOSE:
                handleClose(&dataReceive);
                break;
            case CMD_SETMODE:
                handleSetMode(&dataReceive);
                break;
            case CMD_LSBFIRST:
                handleSetLSBFirst(&dataReceive);
                break;
            case CMD_BITSPERWORD:
                handleSetBitsPerWord(&dataReceive);
                break;
            case CMD_BITSPEED:
                handleSetBitSpeed(&dataReceive);
                break;
            case CMD_SENDRECEIVE:
                handleSendReceive(&dataReceive);
                break;
            case CMD_READ:
                handleReadData(&dataReceive);
                break;
            case CMD_WRITE:
                handleWriteData(&dataReceive);
                break;
            }
        }
    }
}
