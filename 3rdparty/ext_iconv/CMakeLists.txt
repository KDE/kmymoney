set(PREFIX_ext_iconv "${EXTPREFIX}")

ExternalProject_Add(
  ext_iconv
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.15.tar.gz
  URL_MD5 ace8b5f2db42f7b3b3057585e80d9808

  INSTALL_DIR ${PREFIX_ext_iconv}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_iconv} ${GLOBAL_AUTOMAKE_PROFILE}
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
)
