/* config-kmymoney.h.  Generated from config-kmymoney.h.cmake by cmake  */

/* Name of package */
#define PACKAGE "kmymoney"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "kmymoney-devel@kde.org"

/* Define to the full name of this package. */
#define PACKAGE_NAME "KMyMoney"

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H 1

/* The size of `char *', as computed by sizeof. */
#cmakedefine SIZEOF_CHAR_P @SIZEOF_CHAR_P@

/* The size of `int', as computed by sizeof. */
#cmakedefine SIZEOF_INT @SIZEOF_INT@

/* The size of `long', as computed by sizeof. */
#cmakedefine SIZEOF_LONG @SIZEOF_LONG@

/* The size of `short', as computed by sizeof. */
#cmakedefine SIZEOF_SHORT @SIZEOF_SHORT@

/* The size of `size_t', as computed by sizeof. */
#cmakedefine SIZEOF_SIZE_T @SIZEOF_SIZE_T@

/* The size of `unsigned long', as computed by sizeof. */
#cmakedefine SIZEOF_UNSIGNED_LONG @SIZEOF_UNSIGNED_LONG@

#cmakedefine KMM_DEBUG 1

#cmakedefine KMM_DESIGNER 1

/* If we're not using GNU C, elide __attribute__ */
#ifndef __GNUC__
#  define  __attribute__(x)  /*NOTHING*/
#endif
