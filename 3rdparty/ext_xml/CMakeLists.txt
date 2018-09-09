set(PREFIX_ext_xml "${EXTPREFIX}")

ExternalProject_Add(
  ext_xml
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL ftp://xmlsoft.org/libxslt/libxml2-2.9.8.tar.gz
  URL_MD5 b786e353e2aa1b872d70d5d1ca0c740d

  INSTALL_DIR ${PREFIX_ext_xml}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_xml} --host=x86_64-linux-gnu --without-python
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
)
