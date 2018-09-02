SET(PREFIX_ext_gpgme "${EXTPREFIX}" )

ExternalProject_Add(
  ext_gpgme
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://www.gnupg.org/ftp/gcrypt/gpgme/gpgme-1.11.1.tar.bz2
  URL_MD5 129c46fb85a7ffa41e43345e48aee884

  INSTALL_DIR ${PREFIX_ext_gpgme}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_gpgme} --enable-languages=cpp,qt --disable-gpg-test
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
  DEPENDS ext_assuan2
)
