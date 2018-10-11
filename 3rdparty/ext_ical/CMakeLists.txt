ExternalProject_Add(
  ext_ical
  DOWNLOAD_DIR ${EXT_DOWNLOAD_DIR}
  URL https://github.com/libical/libical/archive/v3.0.4.zip
  URL_MD5 cafd73f1e8e55d8139394c99e8ca00b0

  CMAKE_ARGS ${CMAKE_GLOBAL_PROFILE}
             -DUSE_BUILTIN_TZDATA=ON
             -DICAL_UNIX_NEWLINE=OFF
             -DICAL_GLIB=OFF
             -DSHARED_ONLY=ON
)
