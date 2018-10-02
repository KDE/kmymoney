set(PREFIX_ext_sharedmimeinfo "${EXTPREFIX}")

ExternalProject_Add(
  ext_sharedmimeinfo
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://people.freedesktop.org/~hadess/shared-mime-info-1.10.tar.xz
  URL_MD5 418c2ced9dc4dd5ca8b06a755e6d64e9

  INSTALL_DIR ${PREFIX_ext_sharedmimeinfo}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_sharedmimeinfo} ${GLOBAL_AUTOMAKE_PROFILE} ${AUTOMAKE_DEFAULT_HOST} ${AUTOMAKE_SHARED_ONLY} --disable-update-mimedb
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
  DEPENDS ext_intltool
)
