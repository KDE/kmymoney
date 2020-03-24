ExternalProject_Add(
  ext_qt5sqlcipher
  DOWNLOAD_DIR ${EXT_DOWNLOAD_DIR}
  URL https://github.com/blizzard4591/qt5-sqlcipher/archive/v1.0.6.zip
  URL_MD5 4c65b956b862b9baded728cf12eb6179

  CMAKE_ARGS ${CMAKE_GLOBAL_PROFILE}

  DEPENDS ext_sqlcipher
)
