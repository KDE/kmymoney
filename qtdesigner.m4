dnl Macro to check for KDChart include and library files
dnl Availability of KDChart defaults to 'no'

AC_DEFUN([AC_QTDESIGNER_SUPPORT],
[
AC_MSG_CHECKING([if library for Qt-Designer widgets should be installed])
AC_ARG_ENABLE(qtdesigner,
  AC_HELP_STRING([--enable-qtdesigner],[Install KMyMoney specific widget library for Qt-Designer (default=no)]),
  [
    enable_qtdesigner="$enableval" 
    AC_MSG_RESULT($enable_qtdesigner)
  ],
  [
    enable_qtdesigner="no"
    AC_MSG_RESULT($enable_qtdesigner)
  ])
  AM_CONDITIONAL(INSTALL_QTDESIGNER_SUPPORT, test "$enable_qtdesigner" = "yes")
])
