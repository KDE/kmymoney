set(PREFIX_ext_xmlsec1 "${EXTPREFIX}")

# aqbanking checks only for xmlsec1-gnutls >= 1.0.0 so we don't need openssl and nss
ExternalProject_Add(
  ext_xmlsec1
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://www.aleksey.com/xmlsec/download/xmlsec1-1.2.26.tar.gz
  URL_MD5 9c4aaf9ff615a73921b9e3bf4988d878

  INSTALL_DIR ${PREFIX_ext_xmlsec1}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_xmlsec1} ${GLOBAL_AUTOMAKE_PROFILE} --without-openssl --without-nss --disable-crypto-dl
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
  DEPENDS ext_xslt
)
