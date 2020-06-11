ExternalProject_Add(
  ext_sqlcipher
  DOWNLOAD_DIR ${EXT_DOWNLOAD_DIR}

  URL https://github.com/sjemens/qsqlcipher-qt5/archive/v.5.11.3-1.zip    # this has to correspond to ext_qt version
  URL_MD5 89260f50f0c0cdbe3c8803f6bdbb2456

  CONFIGURE_COMMAND <SOURCE_DIR>/qmake DEFINES+=DISABLE_COLUMN_METADATA
  BUILD_COMMAND $(MAKE)
  INSTALL_COMMAND $(MAKE) install

  DEPENDS ext_sqlcipher ext_qt
)
