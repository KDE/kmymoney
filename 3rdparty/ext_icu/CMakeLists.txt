SET(PREFIX_ext_icu "${EXTPREFIX}" )

ExternalProject_Add(
  ext_icu
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL http://download.icu-project.org/files/icu4c/62.1/icu4c-62_1-src.tgz
  URL_MD5 490ad9d920158e0314e10ba74ae9a150

  INSTALL_DIR ${PREFIX_ext_icu}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_icu}  ${GLOBAL_AUTOMAKE_PROFILE}
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
)
