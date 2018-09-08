set(PREFIX_ext_grantlee "${EXTPREFIX}")

ExternalProject_Add(
  ext_grantlee
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://github.com/steveire/grantlee/archive/v5.1.0.zip
  URL_MD5 cf959612f577114ad7c9e3456bc41a62

  INSTALL_DIR ${PREFIX_ext_grantlee}
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_grantlee}
              -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
              ${GLOBAL_PROFILE}
              -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
              -DBUILD_TESTING=false
  UPDATE_COMMAND ""
)
