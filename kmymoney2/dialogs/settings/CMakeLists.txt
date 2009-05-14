INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

ADD_CUSTOM_COMMAND(
  OUTPUT ${KMyMoney2_BINARY_DIR}/kmymoneysettings.h  ${KMyMoney2_BINARY_DIR}/kmymoneysettings.cpp
  DEPENDS ${KMyMoney2_SOURCE_DIR}/kmymoney2.kcfg ${KMyMoney2_SOURCE_DIR}/kmymoneysettings.kcfgc
  COMMAND ${KDE4_KCFGC_EXECUTABLE} -d ${KMyMoney2_BINARY_DIR}/ ${KMyMoney2_SOURCE_DIR}/kmymoney2.kcfg ${KMyMoney2_SOURCE_DIR}/kmymoneysettings.kcfgc
  )


SET (libsettings_a_SOURCES 
	${KMyMoney2_BINARY_DIR}/kmymoneysettings.h 
	ksettingscolors.cpp
	ksettingsfonts.cpp 
  ksettingsforecast.cpp 
  #ksettingsgeneral.cpp
  #ksettingsgpg.cpp 
  #ksettingshome.cpp 
  #ksettingsonlinequotes.cpp
  #ksettingsregister.cpp 
  #ksettingsschedules.cpp 
  )
#missing it
#ksettingsplugins.cpp

SET (libsettings_a_UI 
  ksettingscolorsdecl.ui ksettingsfontsdecl.ui
  ksettingsforecastdecl.ui ksettingsgeneraldecl.ui ksettingsgpgdecl.ui
  ksettingshomedecl.ui ksettingsonlinequotesdecl.ui
  ksettingsregisterdecl.ui ksettingsschedulesdecl.ui
  )

kde4_add_ui_files(libsettings_a_SOURCES ${libsettings_a_UI} )

KDE4_ADD_LIBRARY(settings STATIC ${libsettings_a_SOURCES})
