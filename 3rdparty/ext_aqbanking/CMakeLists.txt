ExternalProject_Add(
  ext_aqbanking
  DOWNLOAD_DIR ${EXT_DOWNLOAD_DIR}
  URL https://www.aquamaniac.de/rdm/attachments/download/283/aqbanking-6.1.4.tar.gz
  URL_MD5 0d18c13d7f6a4e31689422549847d750

  CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${EXT_INSTALL_DIR} --enable-local-install ${AT_HOST} ${AT_SHARED_ONLY} CPPFLAGS=${AT_CPP_FLAGS} LDFLAGS=${AT_LD_FLAGS}
  BUILD_COMMAND $(MAKE)
  INSTALL_COMMAND $(MAKE) install

  DEPENDS ext_gwenhywfar ext_xmlsec1 ext_gmp
)
