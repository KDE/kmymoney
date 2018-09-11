set(PREFIX_ext_jpeg "${EXTPREFIX}")

ExternalProject_Add( ext_jpeg
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://downloads.sourceforge.net/libjpeg-turbo/libjpeg-turbo-2.0.0.tar.gz
  URL_MD5 b12a3fcf1d078db38410f27718a91b83

  INSTALL_DIR ${PREFIX_ext_jpeg}
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_jpeg} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

  UPDATE_COMMAND ""
  DEPENDS ext_nasm
)
