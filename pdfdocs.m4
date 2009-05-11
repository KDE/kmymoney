dnl
dnl check the pdf generation option
dnl if enabled or disabled, directly controlled
dnl
AC_DEFUN([AC_PDF_GENERATION], [
  AC_MSG_CHECKING(if the PDF document generation is desired)
  AC_ARG_ENABLE( pdf-docs,
    [  --enable-pdf-docs       enable generation of PDF documents (default=auto)],
    enable_pdfdocs="$enableval",
    enable_pdfdocs="auto")

  AC_MSG_RESULT($enable_pdfdocs)
  if test "x$enable_pdfdocs" != "xno"; then
    AC_CHECK_PROG(found_recode, recode, yes, no)
    AC_CHECK_PROG(found_html2ps, html2ps, yes, no)
    AC_CHECK_PROG(found_ps2pdf, ps2pdf, yes, no)
    if test "x$found_recode" != "xyes" -o "x$found_html2ps" != "xyes" -o "x$found_ps2pdf" != "xyes"; then
      if test "x$enable_pdfdocs" = "xyes"; then
        AC_MSG_ERROR(At least one of the tools for PDF generation is missing)
      fi
      enable_pdfdocs="no"
    else
      enable_pdfdocs="yes"
    fi
  fi

  AM_CONDITIONAL(GENERATE_PDF, test "x$enable_pdfdocs" = "xyes")
])

