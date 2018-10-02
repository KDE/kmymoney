set(PREFIX_ext_postgresql "${EXTPREFIX}")

set(postgresql_cppFlags "-L${INSTALL_ROOT}/lib -I${INSTALL_ROOT}/include")

ExternalProject_Add(
  ext_postgresql
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://ftp.postgresql.org/pub/source/v10.5/postgresql-10.5.tar.bz2
  URL_MD5 a5fe5fdff2d6c28f65601398be0950df

  INSTALL_DIR ${PREFIX_ext_postgresql}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_postgresql} ${GLOBAL_AUTOMAKE_PROFILE} --with-libxml --with-libxslt --with-openssl CPPFLAGS=${postgresql_cppFlags}
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  UPDATE_COMMAND ""
  BUILD_IN_SOURCE TRUE
  DEPENDS ext_readline
)

# build and install client only
ExternalProject_Add_Step(
ext_postgresql client
COMMAND ${MAKE_EXE} -C src/bin install
COMMAND rsync -a --ignore-existing src/backend/utils/ src/include/utils/
COMMAND ${MAKE_EXE} -C src/include install
COMMAND ${MAKE_EXE} -C src/interfaces install
COMMAND ${MAKE_EXE} -C doc install
DEPENDEES install
WORKING_DIRECTORY <SOURCE_DIR>
)