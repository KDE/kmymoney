SET(PREFIX_ext_gmp "${EXTPREFIX}" )

ExternalProject_Add(
  ext_gmp
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://gmplib.org/download/gmp/gmp-6.1.2.tar.bz2
  URL_MD5 8ddbb26dc3bd4e2302984debba1406a5

  INSTALL_DIR ${PREFIX_ext_gmp}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_gmp} --enable-cxx
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
)
