SET(PREFIX_ext_alkimia "${EXTPREFIX}" )

ExternalProject_Add(
  ext_alkimia
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://download.kde.org/stable/alkimia/7.0.2/alkimia-7.0.2.tar.xz
  URL_MD5 d826e092f56e528c70c00748a04e4957

  INSTALL_DIR ${PREFIX_ext_alkimia}
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_alkimia}
              -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
              ${GLOBAL_PROFILE}
              -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
              -DBUILD_TESTING=false
  UPDATE_COMMAND ""
  DEPENDS ext_gmp
)
