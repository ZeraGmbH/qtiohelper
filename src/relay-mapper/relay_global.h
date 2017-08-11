#ifndef QTRELAYS_GLOBAL_H
#define QTRELAYS_GLOBAL_H

#include <QtCore/qglobal.h>

#ifndef QT_STATIC
#if defined(QT_BUILD_RELAYS_LIB)
#  define QTRELAYSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QTRELAYSSHARED_EXPORT Q_DECL_IMPORT
#endif
#endif

#endif // QTRELAYS_GLOBAL_H
