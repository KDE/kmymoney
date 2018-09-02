SET(PREFIX_ext_opensp "${EXTPREFIX}" )

ExternalProject_Add(
  ext_opensp
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL http://downloads.sourceforge.net/project/openjade/opensp/1.5.2/OpenSP-1.5.2.tar.gz
  URL_MD5 670b223c5d12cee40c9137be86b6c39b

  INSTALL_DIR ${PREFIX_ext_opensp}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_opensp} --enable-shared --disable-static --disable-doc-build
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
)
