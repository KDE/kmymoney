SET(PREFIX_ext_xmlsec1 "${EXTPREFIX}" )

ExternalProject_Add(
  ext_xmlsec1
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://www.aleksey.com/xmlsec/download/xmlsec1-1.2.25.tar.gz
  URL_MD5 dbbef1efc69e61bc4629650205a05b41

  INSTALL_DIR ${PREFIX_ext_xmlsec1}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_xmlsec1} --enable-shared --disable-static --without-openssl --disable-crypto-dl
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
  DEPENDS ext_xslt
)
