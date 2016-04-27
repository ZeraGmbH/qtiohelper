#include <QDataStream>
#include <QEventLoop>
#include <QTimer>
#include <QTcpSocket>

#if defined(QIOHELPERREMOTE_LIBRARY)
#  define QIOHELPERREMOTE_EXPORT Q_DECL_EXPORT
#else
#  define QIOHELPERREMOTE_EXPORT Q_DECL_IMPORT
#endif

bool QIOHELPERREMOTE_EXPORT readTCPFrameBlocked(QTcpSocket *socket, QByteArray *data);
void QIOHELPERREMOTE_EXPORT sendOK(QTcpSocket *socket, bool bOK);
bool QIOHELPERREMOTE_EXPORT receiveOK(QTcpSocket *socket);

