SET(PREFIX_ext_sqlcipher "${EXTPREFIX}" )

ExternalProject_Add(
  ext_sqlcipher
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://github.com/sqlcipher/sqlcipher/archive/v3.4.2.zip
  URL_MD5 f12c5975d784856ea89f83cfc9af1a6c

  INSTALL_DIR ${PREFIX_ext_sqlcipher}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_sqlcipher} --disable-static --enable-shared --enable-tempstore=yes CFLAGS=-DSQLITE_HAS_CODEC
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
  DEPENDS ext_tcl
)
