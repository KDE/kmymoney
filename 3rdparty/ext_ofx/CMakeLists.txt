set(ofx_openSPIncludeDir "${EXT_INSTALL_DIR}/include/OpenSP")
set(ofx_openSPLibDir "${EXT_INSTALL_DIR}/lib")

ExternalProject_Add(
  ext_ofx
  DOWNLOAD_DIR ${EXT_DOWNLOAD_DIR}
  URL http://downloads.sourceforge.net/project/libofx/libofx/libofx-0.9.15.tar.gz
  URL_MD5 2863156ca18f73deb75501a4fc0d6937

  CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${EXT_INSTALL_DIR} ${AT_HOST} ${AT_SHARED_ONLY} --with-opensp-includes=${ofx_openSPIncludeDir} --with-opensp-libs=${ofx_openSPLibDir}
  BUILD_COMMAND $(MAKE)
  INSTALL_COMMAND $(MAKE) install

  DEPENDS ext_opensp
)
