ExternalProject_Add(
  ext_nettle
  DOWNLOAD_DIR ${EXT_DOWNLOAD_DIR}
  URL https://ftp.gnu.org/gnu/nettle/nettle-3.4.tar.gz
  URL_MD5 dc0f13028264992f58e67b4e8915f53d

  CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${EXT_INSTALL_DIR} ${AT_HOST} ${AT_SHARED_ONLY} --enable-public-key --disable-documentation --with-lib-path=${EXT_INSTALL_DIR}/lib --with-include-path=${EXT_INSTALL_DIR}/include
  BUILD_COMMAND $(MAKE)
  INSTALL_COMMAND $(MAKE) install

  DEPENDS ext_gmp
)
