set(PREFIX_ext_ical "${EXTPREFIX}")

ExternalProject_Add(
  ext_ical
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://github.com/libical/libical/archive/v3.0.4.zip
  URL_MD5 cafd73f1e8e55d8139394c99e8ca00b0

  INSTALL_DIR ${PREFIX_ext_ical}
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_ical}
              -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
              ${GLOBAL_PROFILE}
              -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
              -DBUILD_TESTING=false
              -DUSE_BUILTIN_TZDATA=ON
              -DICAL_UNIX_NEWLINE=OFF
              -DICAL_GLIB=OFF
              -DSHARED_ONLY=ON
  UPDATE_COMMAND ""
)
