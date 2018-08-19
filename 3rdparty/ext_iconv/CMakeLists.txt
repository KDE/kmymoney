SET(PREFIX_ext_iconv "${EXTPREFIX}" )

ExternalProject_Add(
  ext_iconv
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL http://files.kde.org/krita/build/dependencies/libiconv-1.14.tar.gz
  URL_MD5 e34509b1623cec449dfeb73d7ce9c6c6

  INSTALL_DIR ${PREFIX_ext_iconv}
  PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/iconv.diff
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_iconv}  ${GLOBAL_AUTOMAKE_PROFILE}
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
)
