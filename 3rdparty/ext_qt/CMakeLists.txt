# qtquickcontrols and qtwebchannel are required by qtwebengine
# "WebRTC" is required by "Pepper Plugins" is required by "Printing and PDF"
# don't use -I and -L switches as they will cause explicit link to
# OpenSSL instead of BoringSSL in QWebEngine
ExternalProject_Add(
  ext_qt
  DOWNLOAD_DIR ${EXT_DOWNLOAD_DIR}
  URL https://ftp.osuosl.org/pub/blfs/conglomeration/qt5/qt-everywhere-src-5.11.3.tar.xz
  URL_MD5 02b353bfe7a40a8dc4274e1d17226d2b

  PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/qsql_mysql.diff

  CONFIGURE_COMMAND <SOURCE_DIR>/configure -prefix ${EXT_INSTALL_DIR} -opensource -confirm-license -verbose -nomake examples -nomake tests -nomake tools ICU_LIBDIR="${EXT_INSTALL_DIR}/lib" ICU_INCDIR="${EXT_INSTALL_DIR}/include" OPENSSL_LIBDIR="${EXT_INSTALL_DIR}/openssl/lib" OPENSSL_INCDIR="${EXT_INSTALL_DIR}/openssl/include" -openssl-linked -skip qt3d -skip qtactiveqt -skip qtcanvas3d -skip qtconnectivity -skip qtenginio -skip qtgraphicaleffects -skip qtlocation -skip qtwayland -skip qtandroidextras -skip qtserialport -skip qtdatavis3d -skip qtvirtualkeyboard -skip qtspeech -skip qtsensors -skip qtgamepad -skip qtscxml -skip qtremoteobjects -skip qtxmlpatterns -skip qtcharts -skip qtdatavis3d -skip qtgamepad -skip qtpurchasing -skip qtscxml -skip qtserialbus -skip qtspeech -skip qtvirtualkeyboard -skip qtdoc -skip qtmacextras -skip qtwinextras -skip qtwebglplugin -skip qtwebsockets -skip qtwebview -skip qtmultimedia -skip qtnetworkauth -skip qtimageformats -system-webengine-icu

  BUILD_COMMAND $(MAKE)
  INSTALL_COMMAND $(MAKE) install
  DEPENDS ext_xslt ext_mysql ext_sqlite ext_postgresql ext_pcre2
)
