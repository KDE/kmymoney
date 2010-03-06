/* config-kmymoney.h.  Generated from config-kmymoney.h.cmake by cmake  */

/* define if you have atoll */
#cmakedefine HAVE_ATOLL 1

/* Define to 1 if you have the <Carbon/Carbon.h> header file. */
#cmakedefine HAVE_CARBON_CARBON_H

/* Define if you have the CoreAudio API */
#cmakedefine HAVE_COREAUDIO 

/* Define to 1 if you have the <crt_externs.h> header file. */
#cmakedefine HAVE_CRT_EXTERNS_H 

/* Defines if your system has the crypt function */
#cmakedefine HAVE_CRYPT 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine HAVE_DLFCN_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H 1

/* Define if you have libkdchart */
#define HAVE_KDCHART 0

/* Define if you have KDChartListTableData::setProp method */
#define HAVE_KDCHART_SETPROP 0

/* Define if you have libcppunit */
#cmakedefine HAVE_LIBCPPUNIT 1

/* Define if you have libjpeg */
#cmakedefine HAVE_LIBJPEG 1

/* Define if you have libpng */
#cmakedefine HAVE_LIBPNG 1

/* Define if you have a working libpthread (will enable threaded code) */
#cmakedefine HAVE_LIBPTHREAD 1

/* Define if you have libz */
#cmakedefine HAVE_LIBZ 1

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H 1

/* Define if your system needs _NSGetEnviron to set up the environment */
#cmakedefine HAVE_NSGETENVIRON 

/* Define if you have res_init */
#cmakedefine HAVE_RES_INIT 1

/* Define if you have the res_init prototype */
#cmakedefine HAVE_RES_INIT_PROTO 1

/* define if you have round */
#cmakedefine HAVE_ROUND 1

/* Define if you have a STL implementation by SGI */
#cmakedefine HAVE_SGI_STL 1

/* Define to 1 if you have the `snprintf' function. */
#cmakedefine HAVE_SNPRINTF 1

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H 1

/* Define if you have strlcat */
#cmakedefine HAVE_STRLCAT 

/* Define if you have the strlcat prototype */
#cmakedefine HAVE_STRLCAT_PROTO 

/* Define if you have strlcpy */
#cmakedefine HAVE_STRLCPY 

/* Define if you have the strlcpy prototype */
#cmakedefine HAVE_STRLCPY_PROTO 

/* define if you have strtoll */
#cmakedefine HAVE_STRTOLL 1

/* Define to 1 if you have the <sys/bitypes.h> header file. */
#cmakedefine HAVE_SYS_BITYPES_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H 1

/* Define to 1 if you have the `vsnprintf' function. */
#cmakedefine HAVE_VSNPRINTF 1

/* Define if unit tests requiring online access should be compiled */
#cmakedefine PERFORM_ONLINE_UNITTESTS 

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

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS 1

/* type to use in place of socklen_t if not defined */
#cmakedefine kde_socklen_t socklen_t

/* type to use in place of socklen_t if not defined (deprecated, use kde_socklen_t) */
#cmakedefine ksize_t socklen_t


#cmakedefine KMM_DEBUG 1

#cmakedefine KMM_DESIGNER 1

/* If we're not using GNU C, elide __attribute__ */
#ifndef __GNUC__
#  define  __attribute__(x)  /*NOTHING*/
#endif
