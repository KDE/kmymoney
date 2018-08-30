SET(PREFIX_ext_gwenhywfar "${EXTPREFIX}" )

ExternalProject_Add(
  ext_gwenhywfar
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://www.aquamaniac.de/sites/download/download.php?package=01&release=208&file=02&dummy=gwenhywfar-4.20.0.tar.gz
  URL_MD5 0744255cae68adca6dff81d3a9f3932d

  INSTALL_DIR ${PREFIX_ext_gwenhywfar}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_gwenhywfar} --with-guis=qt5 --disable-static --enable-shared --disable-binreloc
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
)
