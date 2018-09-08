set(PREFIX_ext_boost "${EXTPREFIX}")

# don't bump boost version without checking it with mysql
# mysql e.g. needs exactly 1.67 and won't work with 1.68
ExternalProject_Add(
  ext_boost
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://dl.bintray.com/boostorg/release/1.67.0/source/boost_1_67_0.tar.bz2
  URL_MD5 ced776cb19428ab8488774e1415535ab

  CONFIGURE_COMMAND <SOURCE_DIR>/bootstrap.sh --prefix=${PREFIX_ext_boost} --with-libraries=system
  BUILD_COMMAND <SOURCE_DIR>/b2 -j${SUBMAKE_JOBS} install
  INSTALL_COMMAND ""
  INSTALL_DIR ${PREFIX_ext_boost}

  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
)
