set(PREFIX_ext_png "${EXTPREFIX}")

ExternalProject_Add( ext_png
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://kent.dl.sourceforge.net/project/libpng/libpng16/1.6.35/libpng-1.6.35.tar.xz
  URL_MD5 678b7e696a62a193ed3503b04bf449d6

  INSTALL_DIR ${PREFIX_ext_png}
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_png} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DPNG_TESTS=OFF

  UPDATE_COMMAND ""
  DEPENDS ext_zlib
)
