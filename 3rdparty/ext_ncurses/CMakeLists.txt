set(PREFIX_ext_ncurses "${EXTPREFIX}")

ExternalProject_Add(
  ext_ncurses
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://ftp.gnu.org/pub/gnu/ncurses/ncurses-6.1.tar.gz
  URL_MD5 98c889aaf8d23910d2b92d65be2e737a

  INSTALL_DIR ${PREFIX_ext_ncurses}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_ncurses} ${GLOBAL_AUTOMAKE_PROFILE}
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
)
