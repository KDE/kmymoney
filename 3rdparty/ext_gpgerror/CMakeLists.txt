set(PREFIX_ext_gpgerror "${EXTPREFIX}")

ExternalProject_Add(
  ext_gpgerror
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://www.gnupg.org/ftp/gcrypt/libgpg-error/libgpg-error-1.32.tar.bz2
  URL_MD5 ef3d928a5a453fa701ecc3bb22be1c64

  INSTALL_DIR ${PREFIX_ext_gpgerror}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_gpgerror} ${GLOBAL_AUTOMAKE_PROFILE}
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
)
