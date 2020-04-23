ExternalProject_Add(
  ext_icu
  DOWNLOAD_DIR ${EXT_DOWNLOAD_DIR}
  URL https://github.com/unicode-org/icu/releases/download/release-63-2/icu4c-63_2-src.tgz
  URL_HASH SHA512=5fa9092efd8d6da6dfc8d498e4026167fda43423eaafc754d1789cf8fd4f6e76377878ebcaa32e14f314836136b764873511a93bfbcc5419b758841cc6df8f32

  CONFIGURE_COMMAND <SOURCE_DIR>/source/configure --prefix=${EXT_INSTALL_DIR} --disable-tests
  BUILD_COMMAND $(MAKE)
  INSTALL_COMMAND $(MAKE) install
)
