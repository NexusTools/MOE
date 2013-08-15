#ifndef SIMPLEX_GLOBAL_H
#define SIMPLEX_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(SIMPLEX_LIBRARY)
#  define SIMPLEXSHARED_EXPORT Q_DECL_EXPORT
#else
#  define SIMPLEXSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // SIMPLEX_GLOBAL_H
