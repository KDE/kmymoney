set (libsettings_a_SOURCES
  ksettingscolors.cpp
  ksettingsfonts.cpp
  ksettingsforecast.cpp
  ksettingsgeneral.cpp
  ksettingsgpg.cpp
  ksettingshome.cpp
  ksettingsregister.cpp
  ksettingsschedules.cpp
  ksettingsplugins.cpp
  ksettingsreports.cpp
  )

set (libsettings_a_UI
  ksettingscolorsdecl.ui ksettingsfontsdecl.ui
  ksettingsforecastdecl.ui ksettingsgeneraldecl.ui ksettingsgpgdecl.ui
  ksettingshomedecl.ui
  ksettingsregisterdecl.ui
  ksettingsschedulesdecl.ui
  ksettingsreportsdecl.ui
  )

kde4_add_ui_files(libsettings_a_SOURCES ${libsettings_a_UI} )

kde4_add_library(settings STATIC ${libsettings_a_SOURCES})
target_link_libraries(settings ${LIBALKIMIA_LIBRARIES})

add_dependencies(settings widgets kmm_settings)
