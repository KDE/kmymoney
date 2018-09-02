SET(PREFIX_ext_gettext "${EXTPREFIX}" )

ExternalProject_Add(
  ext_gettext
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL http://files.kde.org/krita/build/dependencies/gettext-0.19.8.tar.gz
  URL_MD5 e4fffc004f21596becd1055cf36be31d

  INSTALL_DIR ${PREFIX_ext_gettext}
  CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_gettext} --disable-java   ${GLOBAL_AUTOMAKE_PROFILE} --disable-native-java
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  DEPENDS ext_iconv
)
