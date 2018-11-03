ExternalProject_Add(
  ext_grantlee
  DOWNLOAD_DIR ${EXT_DOWNLOAD_DIR}
  URL https://github.com/steveire/grantlee/archive/v5.1.0.zip
  URL_MD5 cf959612f577114ad7c9e3456bc41a62

  CMAKE_ARGS ${CMAKE_GLOBAL_PROFILE}
)
