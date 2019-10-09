set(gwenhywfar_guis "qt5 cpp")

ExternalProject_Add(
  ext_gwenhywfar
  DOWNLOAD_DIR ${EXT_DOWNLOAD_DIR}
  URL https://www.aquamaniac.de/rdm/attachments/download/140/gwenhywfar-4.99.16beta.tar.gz
  URL_MD5 be67ed26f2e83cb44e4d4abb7bec467b

  CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${EXT_INSTALL_DIR} --enable-local-install ${AT_SHARED_ONLY} --with-guis=${gwenhywfar_guis} CPPFLAGS=${AT_CPP_FLAGS} LDFLAGS=${AT_LD_FLAGS}
  BUILD_COMMAND $(MAKE)
  INSTALL_COMMAND $(MAKE) install

  DEPENDS ext_gnutls ext_gcrypt
)
