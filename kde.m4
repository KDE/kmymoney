dnl
dnl AM_KDE_MIN_VERSION(MIN-VERSION-MAJOR, MIN-VERSION-MINOR, MIN-VERSION-MICRO)
dnl
AC_DEFUN([AM_KDE_MIN_VERSION],
[
	AC_MSG_CHECKING([if minimum KDE version is available])
	AC_LANG_SAVE
	AC_LANG_CPLUSPLUS
	save_CXXFLAGS=$CXXFLAGS
	CXXFLAGS="$CXXFLAGS -I$srcdir $all_includes"
	AC_TRY_COMPILE([
	#include "kdecompat.h"
	#if !( KDE_IS_VERSION( $1, $2, $3 ) )
	#error KDE version does not meet KMyMoney minimum requirement
	#endif
		], [], AC_MSG_RESULT(yes), AC_MSG_ERROR(no)
	)
	CXXFLAGS=$save_CXXFLAGS
	AC_LANG_RESTORE
])

