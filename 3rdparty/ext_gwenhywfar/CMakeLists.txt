set(gwenhywfar_guis "qt5 cpp")

ExternalProject_Add(
  ext_gwenhywfar
  DOWNLOAD_DIR ${EXT_DOWNLOAD_DIR}
  URL https://www.aquamaniac.de/rdm/attachments/download/331/gwenhywfar-5.4.0.tar.gz
  URL_MD5 af3d547d46920964f25af853b9efc9f1

  CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${EXT_INSTALL_DIR} --enable-local-install ${AT_SHARED_ONLY} --with-guis=${gwenhywfar_guis} CPPFLAGS=${AT_CPP_FLAGS} LDFLAGS=${AT_LD_FLAGS}
  BUILD_COMMAND $(MAKE)
  INSTALL_COMMAND $(MAKE) install

  DEPENDS ext_gnutls ext_gcrypt
)
