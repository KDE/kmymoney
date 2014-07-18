ADD_CUSTOM_COMMAND(
  OUTPUT ${KMyMoney2_BINARY_DIR}/kmymoneysettings.h  ${KMyMoney2_BINARY_DIR}/kmymoneysettings.cpp
  DEPENDS ${KMyMoney2_SOURCE_DIR}/kmymoney.kcfg ${KMyMoney2_SOURCE_DIR}/kmymoneysettings.kcfgc
  COMMAND ${KDE4_KCFGC_EXECUTABLE} -d ${KMyMoney2_BINARY_DIR}/ ${KMyMoney2_SOURCE_DIR}/kmymoney.kcfg ${KMyMoney2_SOURCE_DIR}/kmymoneysettings.kcfgc
  )


SET (libsettings_a_SOURCES 
	${KMyMoney2_BINARY_DIR}/kmymoneysettings.h 
	ksettingscolors.cpp
	ksettingsfonts.cpp 
	ksettingsforecast.cpp 
	ksettingsgeneral.cpp
	ksettingsgpg.cpp 
	ksettingshome.cpp 
	ksettingsonlinequotes.cpp
	ksettingsregister.cpp 
	ksettingsschedules.cpp 
	ksettingsplugins.cpp
	ksettingsreports.cpp
	)

SET (libsettings_a_UI 
  ksettingscolorsdecl.ui ksettingsfontsdecl.ui
  ksettingsforecastdecl.ui ksettingsgeneraldecl.ui ksettingsgpgdecl.ui
  ksettingshomedecl.ui ksettingsonlinequotesdecl.ui
  ksettingsregisterdecl.ui ksettingsschedulesdecl.ui
  ksettingsreportsdecl.ui
  )

qt5_wrap_ui(libsettings_a_SOURCES ${libsettings_a_UI} )

add_library(settings STATIC ${libsettings_a_SOURCES})

target_link_libraries(settings PUBLIC KF5::KCMUtils KF5::ItemViews KF5::Holidays Alkimia::alkimia)

ADD_DEPENDENCIES(settings widgets kmm_config)
