set(PREFIX_ext_tcl "${EXTPREFIX}")

ExternalProject_Add(
  ext_tcl
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://github.com/tcltk/tcl/archive/core-8-6-8.zip
  URL_MD5 8fd508d3d2216b9c75895d6141219eab

  INSTALL_DIR ${PREFIX_ext_tcl}
  CONFIGURE_COMMAND ./unix/configure --prefix=${PREFIX_ext_tcl} ${GLOBAL_AUTOMAKE_PROFILE} --enable-threads --enable-64bit
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
)
