set(PREFIX_ext_ncurses "${EXTPREFIX}")

ExternalProject_Add(
  ext_ncurses
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://ftp.gnu.org/pub/gnu/ncurses/ncurses-6.1.tar.gz
  URL_MD5 98c889aaf8d23910d2b92d65be2e737a

  INSTALL_DIR ${PREFIX_ext_ncurses}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_ncurses} --enable-overwrite --enable-pc-files --without-tests --with-shared --with-cxx-shared --without-ada --with-pkg-config-libdir=${INSTALL_ROOT}/lib/pkgconfig
  BUILD_COMMAND ${MAKE_EXE}
  INSTALL_COMMAND ${MAKE_EXE} install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
)
