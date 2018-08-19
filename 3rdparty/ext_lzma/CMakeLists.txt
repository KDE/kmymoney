SET(PREFIX_ext_lzma "${EXTPREFIX}" )

ExternalProject_Add(
  ext_lzma
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL http://tukaani.org/xz/xz-5.2.3.tar.xz
  URL_MD5 60fb79cab777e3f71ca43d298adacbd5

  INSTALL_DIR ${PREFIX_ext_lzma}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_lzma}  ${GLOBAL_AUTOMAKE_PROFILE}
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
)
