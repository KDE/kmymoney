set(PREFIX_ext_lzma "${EXTPREFIX}")

ExternalProject_Add(
  ext_lzma
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL http://tukaani.org/xz/xz-5.2.4.tar.xz
  URL_MD5 003e4d0b1b1899fc6e3000b24feddf7c

  INSTALL_DIR ${PREFIX_ext_lzma}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_lzma}
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
)
