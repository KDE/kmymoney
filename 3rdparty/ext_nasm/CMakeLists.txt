set(PREFIX_ext_nasm "${EXTPREFIX}")

ExternalProject_Add(
  ext_nasm
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://www.nasm.us/pub/nasm/releasebuilds/2.13.03/nasm-2.13.03.tar.xz
  URL_MD5 d5ca2ad7121ccbae69dd606b1038532c

  INSTALL_DIR ${PREFIX_ext_nasm}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_nasm} ${GLOBAL_AUTOMAKE_PROFILE}
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
)
