set(PREFIX_ext_ktoblzcheck "${EXTPREFIX}")

ExternalProject_Add(
  ext_ktoblzcheck
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL http://downloads.sourceforge.net/project/ktoblzcheck/ktoblzcheck-1.49.tar.gz
  URL_MD5 2ea1b01a931eb1e52b0ca0e07cb72435

  INSTALL_DIR ${PREFIX_ext_ktoblzcheck}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_ktoblzcheck} ${GLOBAL_AUTOMAKE_PROFILE}
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
)
