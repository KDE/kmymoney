ExternalProject_Add(
  ext_openssl
  DOWNLOAD_DIR ${EXT_DOWNLOAD_DIR}
  URL https://www.openssl.org/source/openssl-1.1.0i.tar.gz
  URL_MD5 9495126aafd2659d357ea66a969c3fe1

  CONFIGURE_COMMAND <SOURCE_DIR>/config --prefix=${EXT_INSTALL_DIR} --openssldir=${EXT_INSTALL_DIR}/ssl
  BUILD_COMMAND $(MAKE)
  INSTALL_COMMAND $(MAKE) install
)
