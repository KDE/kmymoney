set(PREFIX_ext_gettext "${EXTPREFIX}")

ExternalProject_Add(
  ext_gettext
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL http://ftp.gnu.org/pub/gnu/gettext/gettext-0.19.8.1.tar.gz
  URL_MD5 97e034cf8ce5ba73a28ff6c3c0638092

  INSTALL_DIR ${PREFIX_ext_gettext}
  CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_gettext} ${GLOBAL_AUTOMAKE_PROFILE} --disable-java --disable-native-java --enable-nls --enable-c++
  BUILD_COMMAND make
  INSTALL_COMMAND make install

  UPDATE_COMMAND ""
  DEPENDS ext_iconv ext_xml ext_ncurses
)
