set(PREFIX_ext_xslt "${EXTPREFIX}")

ExternalProject_Add(
  ext_xslt
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL ftp://xmlsoft.org/libxslt/libxslt-1.1.32.tar.gz
  URL_MD5 1fc72f98e98bf4443f1651165f3aa146

  INSTALL_DIR ${PREFIX_ext_xslt}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_xslt} --host=x86_64-linux-gnu
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
  DEPENDS ext_xml
)
