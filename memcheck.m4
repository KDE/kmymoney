dnl
dnl check the memory leakage checker option
dnl if enabled or disabled, directly controlled
dnl if not given, follows --enable-debug and if
dnl debugging support is turned on, the memory
dnl leakage checker is turned on also
dnl
dnl Need AC_CHECK_COMPILERS to be run before
AC_DEFUN([AC_MEMORY_LEAK_CHECK], [
  AC_ARG_ENABLE(
  leak-check,
  AC_HELP_STRING([--enable-leak-check],[enable memory leak checker (default=no)]),
  use_memory_leak_check=$enableval,use_memory_leak_check=no)

  if test "x$use_memory_leak_check" != "xno"; then
    CPPFLAGS="$CPPFLAGS -D_CHECK_MEMORY"
  fi
  if test "x$kde_use_debug_code" != "xno"; then
    CPPFLAGS="$CPPFLAGS -DKMM_DEBUG=1"
  else
    CPPFLAGS="$CPPFLAGS -DKMM_DEBUG=0"
  fi
])

