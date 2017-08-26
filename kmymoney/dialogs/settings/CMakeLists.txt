add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kmymoneysettings.h  ${CMAKE_CURRENT_BINARY_DIR}/kmymoneysettings.cpp
  DEPENDS ${CMAKE_SOURCE_DIR}/kmymoney/kmymoney.kcfg ${CMAKE_SOURCE_DIR}/kmymoney/kmymoneysettings.kcfgc
  COMMAND ${KDE4_KCFGC_EXECUTABLE} -d ${CMAKE_CURRENT_BINARY_DIR}/ ${CMAKE_SOURCE_DIR}/kmymoney/kmymoney.kcfg ${CMAKE_SOURCE_DIR}/kmymoney/kmymoneysettings.kcfgc
  )

set (libsettings_a_SOURCES
  ${CMAKE_CURRENT_BINARY_DIR}/kmymoneysettings.cpp
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

kde4_add_ui_files(libsettings_a_SOURCES ${libsettings_a_UI} )

kde4_add_library(settings STATIC ${libsettings_a_SOURCES})

add_dependencies(settings widgets kmm_config)
