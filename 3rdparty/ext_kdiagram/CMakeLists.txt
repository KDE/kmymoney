set(PREFIX_ext_kdiagram "${EXTPREFIX}")

ExternalProject_Add(
  ext_kdiagram
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://download.kde.org/stable/kdiagram/2.6.1/kdiagram-2.6.1.tar.xz
  URL_MD5 d0f8624a2decad2ba6a871e13799da23

  INSTALL_DIR ${PREFIX_ext_kdiagram}
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_kdiagram}
              -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
              ${GLOBAL_PROFILE}
              -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
              -DBUILD_TESTING=false
  UPDATE_COMMAND ""
)
