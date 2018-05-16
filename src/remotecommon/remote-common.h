#ifndef QTREMOTECOMMON_H
#define QTREMOTECOMMON_H

#include <QtCore/qglobal.h>
#include <QTcpSocket>

#if defined(QT_BUILD_REMOTECOMMON_LIB)
#  define QREMOTECOMMON_EXPORT Q_DECL_EXPORT
#else
#  define QREMOTECOMMON_EXPORT Q_DECL_IMPORT
#endif

bool QREMOTECOMMON_EXPORT readTCPFrameBlocked(QTcpSocket *socket, QByteArray *data);
void QREMOTECOMMON_EXPORT sendOK(QTcpSocket *socket, bool bOK);
bool QREMOTECOMMON_EXPORT receiveOK(QTcpSocket *socket);


#endif // QTREMOTECOMMON_H
