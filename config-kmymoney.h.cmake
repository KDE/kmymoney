/* config-kmymoney.h.  Generated from config-kmymoney.h.cmake by cmake  */

/* Name of package */
#define PACKAGE "kmymoney"

#cmakedefine KMM_DESIGNER 1
/* No external component */

#cmakedefine KMM_DBUS 1
/* No external component */

#cmakedefine ENABLE_HOLIDAYS 1
/* Part of KF */

#cmakedefine ENABLE_ADDRESSBOOK 1
#define ENABLE_ADDRESSBOOK_VERSION "@ADDRESSBOOK_VERSION@"

#cmakedefine ENABLE_ACTIVITIES 1
/* Part of KF */

#cmakedefine ENABLE_KBANKING 1
#define ENABLE_AQBANKING_VERSION "@aqbanking_VERSION@"
#define ENABLE_GWENHYWFAR_VERSION "@gwenhywfar_VERSION@"

/* Required package */
#define ENABLE_KCHART_VERSION "@KChart_VERSION@"

#cmakedefine ENABLE_LIBICAL 1
#define ENABLE_LIBICAL_VERSION "@LibIcal_VERSION@"

#cmakedefine ENABLE_LIBOFX 1
#define ENABLE_LIBOFX_VERSION "@libofx_VERSION@"

#cmakedefine ENABLE_SQLCIPHER 1
#define ENABLE_SQLCIPHER_VERSION "@SQLCIPHER_VERSION@"

#cmakedefine ENABLE_SQLTRACER 1
/* No external component */

#cmakedefine ENABLE_GPG 1
#define ENABLE_GPG_VERSION "@GPG_VERSION@"
#cmakedefine HAVE_GPGMEPP_AS_STD_STRING 1

#cmakedefine ENABLE_COSTCENTER 1
/* No external component */

#include <qglobal.h>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define QLIST_COUNT_FORMAT "%lld"
#else
#define QLIST_COUNT_FORMAT "%d"
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define QT6_IF(a, b) a
#else
#define QT6_IF(a, b) b
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define KMM_QT6_ONLY(a) a
#define KNN_QT5_ONLY(a)
#else
#define KMM_QT5_ONLY(a) a
#define KMM_QT6_ONLY(a)
#endif
