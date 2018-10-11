ExternalProject_Add(
  ext_kdiagram
  DOWNLOAD_DIR ${EXT_DOWNLOAD_DIR}
  URL https://download.kde.org/stable/kdiagram/2.6.1/kdiagram-2.6.1.tar.xz
  URL_MD5 d0f8624a2decad2ba6a871e13799da23

  CMAKE_ARGS ${CMAKE_GLOBAL_PROFILE}
)
