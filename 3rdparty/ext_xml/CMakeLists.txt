SET(PREFIX_ext_xml "${EXTPREFIX}" )

ExternalProject_Add(
  ext_xml
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL ftp://xmlsoft.org/libxslt/libxml2-2.9.7.tar.gz
  URL_MD5 896608641a08b465098a40ddf51cefba

  INSTALL_DIR ${PREFIX_ext_xml}
  PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/liconv.diff
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_xml} --without-python
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
)
