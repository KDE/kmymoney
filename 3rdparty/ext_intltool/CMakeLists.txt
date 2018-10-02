set(PREFIX_ext_intltool "${EXTPREFIX}")

ExternalProject_Add(
  ext_intltool
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://launchpad.net/intltool/trunk/0.51.0/+download/intltool-0.51.0.tar.gz
  URL_MD5 12e517cac2b57a0121cda351570f1e63

  INSTALL_DIR ${PREFIX_ext_intltool}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_intltool} ${GLOBAL_AUTOMAKE_PROFILE} ${AUTOMAKE_DEFAULT_HOST} ${AUTOMAKE_SHARED_ONLY}
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
)
