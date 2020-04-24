# aqbanking checks only for xmlsec1-gnutls >= 1.0.0 so we don't need openssl and nss
# we need gcrypt though because xmlsec-gnutls library requires xmlsec-gcrypt
ExternalProject_Add(
  ext_xmlsec1
  DOWNLOAD_DIR ${EXT_DOWNLOAD_DIR}
  URL https://www.aleksey.com/xmlsec/download/xmlsec1-1.2.30.tar.gz
  URL_HASH SHA512=07152470a9fe5d077f8a358608ca1d8a79ee0d2777660f61ed5717dc640714a3adfe66843e6a4023898eb0f5ed79771d70c41132571f3a1aeda82c1894b69c98

  CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${EXT_INSTALL_DIR} ${AT_HOST} ${AT_SHARED_ONLY} --without-openssl --without-nss --disable-crypto-dl
  BUILD_COMMAND $(MAKE)
  INSTALL_COMMAND $(MAKE) install

  DEPENDS ext_xslt ext_gcrypt ext_gnutls
)
