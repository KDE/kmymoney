set(gwenhywfar_guis "qt5 cpp")

ExternalProject_Add(
  ext_gwenhywfar
  DOWNLOAD_DIR ${EXT_DOWNLOAD_DIR}
  URL https://www.aquamaniac.de/rdm/attachments/download/377/gwenhywfar-5.7.1.tar.gz
  URL_MD5 0163d800afd6b2c264522eedc0dde37b

  CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${EXT_INSTALL_DIR} --enable-local-install ${AT_SHARED_ONLY} --with-guis=${gwenhywfar_guis} CPPFLAGS=${AT_CPP_FLAGS} LDFLAGS=${AT_LD_FLAGS}
  BUILD_COMMAND $(MAKE)
  INSTALL_COMMAND $(MAKE) install

  DEPENDS ext_gnutls ext_gcrypt
)
