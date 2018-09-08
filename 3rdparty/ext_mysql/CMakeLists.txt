set(PREFIX_ext_mysql "${EXTPREFIX}")

ExternalProject_Add(
  ext_mysql
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://cdn.mysql.com//Downloads/MySQL-8.0/mysql-8.0.12.tar.gz
  URL_MD5 569b6037bad533ad84c2b8d29534a68f

  INSTALL_DIR ${PREFIX_ext_mysql}
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_mysql}
              -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
              ${GLOBAL_PROFILE}
              -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
              -DBUILD_TESTING=false
              -DBUILD_CONFIG=mysql_release
              -DCPACK_MONOLITHIC_INSTALL=1
              -DIGNORE_AIO_CHECK=1
#               -DDOWNLOAD_BOOST=ON
#               -DWITH_BOOST=${EXTERNALS_DOWNLOAD_DIR}/boost-1.67
#               -DWITH_ZLIB=system
#               -DWITH_LZMA=system
#               -DINSTALL_STATIC_LIBRARIES=OFF

  UPDATE_COMMAND ""
  DEPENDS ext_icu ext_ncurses
)
