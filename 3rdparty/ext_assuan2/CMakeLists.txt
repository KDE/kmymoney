set(PREFIX_ext_assuan2 "${EXTPREFIX}")

ExternalProject_Add(
  ext_assuan2
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://www.gnupg.org/ftp/gcrypt/libassuan/libassuan-2.5.1.tar.bz2
  URL_MD5 4354b7ae296894f232ada226a062d7d7

  INSTALL_DIR ${PREFIX_ext_assuan2}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_assuan2} ${GLOBAL_AUTOMAKE_PROFILE}
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
  DEPENDS ext_gpgerror
)
