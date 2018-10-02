set(PREFIX_ext_mysql "${EXTPREFIX}")

ExternalProject_Add(
  ext_mysql
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://cdn.mysql.com//Downloads/MySQL-8.0/mysql-boost-8.0.12.tar.gz
  URL_MD5 82ba7cc22755f77635768c6e6d184161

  INSTALL_DIR ${PREFIX_ext_mysql}
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_mysql}
              -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
              ${GLOBAL_PROFILE}
              -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
              -DWITH_BOOST=<SOURCE_DIR>/boost
              -DBUILD_CONFIG=mysql_release
#               -DCPACK_MONOLITHIC_INSTALL=1
#               -DIGNORE_AIO_CHECK=1
#               -DINSTALL_STATIC_LIBRARIES=OFF

  BUILD_COMMAND "" # on default whole server would be built
  INSTALL_COMMAND ""
  DEPENDS ext_ncurses
)

# build and install client only
ExternalProject_Add_Step(
ext_mysql client
COMMAND ${CMAKE_MAKE_PROGRAM}
COMMAND ${CMAKE_MAKE_PROGRAM} install
DEPENDEES install
WORKING_DIRECTORY <BINARY_DIR>/libmysql
)

ExternalProject_Add_Step(
ext_mysql scripts
COMMAND ${CMAKE_MAKE_PROGRAM}
COMMAND ${CMAKE_MAKE_PROGRAM} install
DEPENDEES client
WORKING_DIRECTORY <BINARY_DIR>/scripts
)

# build and install headers
ExternalProject_Add_Step(
ext_mysql headers
COMMAND ${CMAKE_MAKE_PROGRAM}
COMMAND ${CMAKE_MAKE_PROGRAM} install
DEPENDEES scripts
WORKING_DIRECTORY <BINARY_DIR>/include)