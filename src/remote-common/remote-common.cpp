#include "remote-common.h"

bool readTCPFrameBlocked(QTcpSocket *socket, QByteArray *data)
{
    data->clear();
    // we have to read leading length first
    quint32 ui32Count = sizeof(ui32Count);
    QDataStream stream(data, QIODevice::ReadOnly);
    bool bLenValid = false;
    while ((quint32)data->size() < ui32Count)
    {
        socket->waitForReadyRead();
        *data += socket->read(qMin(ui32Count, (quint32)socket->bytesAvailable()));
        if(!bLenValid && (quint32)data->size() >= sizeof(ui32Count))
        {
            stream >> ui32Count;
            bLenValid = true;
            // we just read count there might be more waiting
            *data += socket->read(qMin(ui32Count, (quint32)socket->bytesAvailable()));
        }
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

