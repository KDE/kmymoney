ExternalProject_Add(
  ext_icu
  DOWNLOAD_DIR ${EXT_DOWNLOAD_DIR}
  URL http://download.icu-project.org/files/icu4c/62.1/icu4c-62_1-src.tgz
  URL_MD5 490ad9d920158e0314e10ba74ae9a150

  CONFIGURE_COMMAND <SOURCE_DIR>/source/configure --prefix=${EXT_INSTALL_DIR} --disable-tests
  BUILD_COMMAND $(MAKE)
  INSTALL_COMMAND $(MAKE) install
)
