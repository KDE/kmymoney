set(PREFIX_ext_ofx "${EXTPREFIX}")
set(ofx_openSPIncludeDir "${INSTALL_ROOT}/include/OpenSP")
set(ofx_openSPLibDir "${INSTALL_ROOT}/lib")

ExternalProject_Add(
  ext_ofx
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL http://downloads.sourceforge.net/project/libofx/libofx/libofx-0.9.13.tar.gz
  URL_MD5 dde6276ffe2999f86bed5bf2cc80f7b2

  INSTALL_DIR ${PREFIX_ext_ofx}
  CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_ofx} ${GLOBAL_AUTOMAKE_PROFILE} --with-opensp-includes=${ofx_openSPIncludeDir} --with-opensp-libs=${ofx_openSPLibDir}
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
  DEPENDS ext_opensp
)
