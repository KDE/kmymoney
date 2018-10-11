ExternalProject_Add(
  ext_postgresql
  DOWNLOAD_DIR ${EXT_DOWNLOAD_DIR}
  URL https://ftp.postgresql.org/pub/source/v10.5/postgresql-10.5.tar.bz2
  URL_MD5 a5fe5fdff2d6c28f65601398be0950df

  CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${EXT_INSTALL_DIR} ${AT_HOST} ${AT_SHARED_ONLY} --with-libxml --with-libxslt --with-openssl --with-icu CPPFLAGS=${AT_CPP_FLAGS}
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
  DEPENDS ext_readline
)

# build and install client only
ExternalProject_Add_Step(
ext_postgresql client
COMMAND $(MAKE) -C src/bin install
COMMAND rsync -a --ignore-existing src/backend/utils/ src/include/utils/
COMMAND $(MAKE) -C src/include install
COMMAND $(MAKE) -C src/interfaces install
COMMAND $(MAKE) -C doc install
DEPENDEES install
WORKING_DIRECTORY <SOURCE_DIR>
)
