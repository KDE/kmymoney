set(PREFIX_ext_sqlite "${EXTPREFIX}")

ExternalProject_Add(
  ext_sqlite
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://sqlite.org/2018/sqlite-amalgamation-3240000.zip
  URL_MD5 4ea1e0c6e7e82cb0490d4753d6878698

  PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/sqlite.diff
  INSTALL_DIR ${PREFIX_ext_sqlite}
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_sqlite}
              -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
              ${GLOBAL_PROFILE}
              -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
              -DBUILD_TESTING=false
              -DSQLITE_BUILD_STATIC=FALSE
              -DENABLE_UNLOCK_NOTIFY=TRUE
  UPDATE_COMMAND ""
)
