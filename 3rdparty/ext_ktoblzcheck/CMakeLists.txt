ExternalProject_Add(
  ext_ktoblzcheck
  DOWNLOAD_DIR ${EXT_DOWNLOAD_DIR}
  URL http://downloads.sourceforge.net/project/ktoblzcheck/ktoblzcheck-1.49.tar.gz
  URL_MD5 2ea1b01a931eb1e52b0ca0e07cb72435

  CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${EXT_INSTALL_DIR} ${AT_HOST} ${AT_SHARED_ONLY}
  BUILD_COMMAND $(MAKE)
  INSTALL_COMMAND $(MAKE) install
)
