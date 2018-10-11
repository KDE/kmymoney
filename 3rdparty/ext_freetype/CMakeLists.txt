ExternalProject_Add(
  ext_freetype
	DOWNLOAD_DIR ${EXT_DOWNLOAD_DIR}
	URL http://files.kde.org/krita/build/dependencies/freetype-2.6.5.tar.gz
  URL_MD5 31b2276515d9ee1c7f37d9c9f4f3145a

  CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${EXT_INSTALL_DIR} ${AT_HOST} ${AT_SHARED_ONLY}
  BUILD_COMMAND $(MAKE)
  INSTALL_COMMAND $(MAKE) install
)
