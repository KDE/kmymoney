set(PREFIX_ext_sqlite "${EXTPREFIX}")

set(sqlite_cFlags "-DSQLITE_ENABLE_COLUMN_METADATA=1 -DSQLITE_DISABLE_DIRSYNC=1 -DSQLITE_SECURE_DELETE=1 -DSQLITE_ENABLE_UNLOCK_NOTIFY=1 -DSQLITE_ENABLE_DBSTAT_VTAB=1 -DSQLITE_ENABLE_FTS3_PARENTHESIS=1")

ExternalProject_Add(
  ext_sqlite
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://www.sqlite.org/2018/sqlite-autoconf-3250200.tar.gz
  URL_MD5 bfade31d59f58badc51aeaa6ae26a5de

  INSTALL_DIR ${PREFIX_ext_sqlite}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_sqlite} --disable-static --enable-shared --enable-fts3 --disable-static-shell --enable-session --enable-json1 CFLAGS=${sqlite_cFlags}
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
)
