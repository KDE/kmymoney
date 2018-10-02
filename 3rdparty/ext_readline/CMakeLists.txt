set(PREFIX_ext_readline "${EXTPREFIX}")

ExternalProject_Add(
  ext_readline
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://ftp.gnu.org/gnu/readline/readline-7.0.tar.gz
  URL_MD5 205b03a87fc83dab653b628c59b9fc91

  INSTALL_DIR ${PREFIX_ext_readline}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_readline} ${GLOBAL_AUTOMAKE_PROFILE} --with-curses
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
  DEPENDS ext_ncurses
)
