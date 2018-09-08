set(PREFIX_ext_aqbanking "${EXTPREFIX}")
set(aqbanking_cppFlags "-L${INSTALL_ROOT}/lib -I${INSTALL_ROOT}/include")

ExternalProject_Add(
  ext_aqbanking
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://www.aquamaniac.de/sites/download/download.php?package=03&release=217&file=02&dummy=aqbanking-5.7.8.tar.gz
  URL_MD5 6aceeaad3a1000f22f08986e68946db3

  INSTALL_DIR ${PREFIX_ext_aqbanking}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_aqbanking} ${GLOBAL_AUTOMAKE_PROFILE} CPPFLAGS=${aqbanking_cppFlags}
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
  DEPENDS ext_gwenhywfar ext_ktoblzcheck ext_xmlsec1 ext_gmp
)
