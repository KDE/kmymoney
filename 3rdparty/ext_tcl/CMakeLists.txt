ExternalProject_Add(
  ext_tcl
  DOWNLOAD_DIR ${EXT_DOWNLOAD_DIR}
  URL https://github.com/tcltk/tcl/archive/core-8-6-8.zip
  URL_MD5 b018a409832df1788f22b1a983fd7c5b

  CONFIGURE_COMMAND <SOURCE_DIR>/unix/configure --prefix=${EXT_INSTALL_DIR} ${AT_HOST} ${AT_SHARED_ONLY} --enable-threads --enable-64bit
  BUILD_COMMAND $(MAKE)
  INSTALL_COMMAND $(MAKE) install
)
