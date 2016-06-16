add_custom_command(
  OUTPUT ${KMyMoney2_BINARY_DIR}/kmymoneysettings.h  ${KMyMoney2_BINARY_DIR}/kmymoneysettings.cpp
  DEPENDS ${KMyMoney2_SOURCE_DIR}/kmymoney.kcfg ${KMyMoney2_SOURCE_DIR}/kmymoneysettings.kcfgc
  COMMAND ${KDE4_KCFGC_EXECUTABLE} -d ${KMyMoney2_BINARY_DIR}/ ${KMyMoney2_SOURCE_DIR}/kmymoney.kcfg ${KMyMoney2_SOURCE_DIR}/kmymoneysettings.kcfgc
  )


set (libsettings_a_SOURCES
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

set (libsettings_a_UI
  ksettingscolorsdecl.ui ksettingsfontsdecl.ui
  ksettingsforecastdecl.ui ksettingsgeneraldecl.ui ksettingsgpgdecl.ui
  ksettingshomedecl.ui ksettingsonlinequotesdecl.ui
  ksettingsregisterdecl.ui ksettingsschedulesdecl.ui
  ksettingsreportsdecl.ui
  )

qt5_wrap_ui(libsettings_a_SOURCES ${libsettings_a_UI} )

add_library(settings STATIC ${libsettings_a_SOURCES})

# TODO: cleanup dependencies
target_link_libraries(settings PUBLIC KF5::Completion KF5::KIOWidgets KF5::TextWidgets KF5::I18n KF5::IconThemes KF5::Completion KF5::KCMUtils KF5::ItemViews Alkimia::alkimia Qt5::Sql)

if (KF5Holidays_FOUND)
  target_link_libraries(settings PUBLIC KF5::Holidays)
endif()

add_dependencies(settings widgets kmm_config)
