SET(EXTPREFIX_qt  "${EXTPREFIX}")

ExternalProject_Add(
  ext_qt
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://download.qt.io/official_releases/qt/5.10/5.10.0/single/qt-everywhere-src-5.10.0.tar.xz
  URL_MD5 c5e275ab0ed7ee61d0f4b82cd471770d

  CONFIGURE_COMMAND <SOURCE_DIR>/configure -prefix ${EXTPREFIX_qt} -opensource -confirm-license -verbose -nomake examples -skip qt3d -skip qtactiveqt -skip qtcanvas3d -skip qtconnectivity -skip qtenginio -skip qtgraphicaleffects -skip qtlocation -skip qtwayland -skip qtandroidextras -skip qtserialport -skip qtdatavis3d -skip qtvirtualkeyboard -skip qtspeech -skip qtsensors -skip qtgamepad -skip qtscxml -skip qtremoteobjects -skip qtxmlpatterns -skip qtcharts -skip qtdatavis3d -skip qtgamepad -skip qtpurchasing -skip qtscxml -skip qtserialbus -skip qtspeech -skip qtvirtualkeyboard
  INSTALL_DIR ${EXTPREFIX_qt}

  BUILD_COMMAND $(MAKE)
  INSTALL_COMMAND $(MAKE) install
  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
)
