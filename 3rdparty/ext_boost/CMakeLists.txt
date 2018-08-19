SET(PREFIX_ext_boost "${EXTPREFIX}" )

ExternalProject_Add(
  ext_boost
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL http://files.kde.org/krita/build/dependencies/boost_1_61_0.tar.bz2
  URL_MD5 6095876341956f65f9d35939ccea1a9f

  CONFIGURE_COMMAND <SOURCE_DIR>/bootstrap.sh --prefix=${PREFIX_ext_boost} --with-libraries=system
  BUILD_COMMAND <SOURCE_DIR>/b2 -j${SUBMAKE_JOBS} install
  INSTALL_COMMAND ""
  INSTALL_DIR ${PREFIX_ext_boost}

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
)
