ExternalProject_Add(
  ext_grantlee
  DOWNLOAD_DIR ${EXT_DOWNLOAD_DIR}
  URL https://github.com/steveire/grantlee/archive/v5.2.0.zip
  URL_MD5 917e8fc0e5a244191cac69a25cea36b7

  CMAKE_ARGS ${CMAKE_GLOBAL_PROFILE}
)
