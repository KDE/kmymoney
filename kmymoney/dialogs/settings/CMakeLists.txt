
set (libsettings_a_SOURCES
  ksettingscolors.cpp
  ksettingsfonts.cpp
  ksettingsicons.cpp
  ksettingsforecast.cpp
  ksettingsgeneral.cpp
  ksettingsgpg.cpp
  ksettingshome.cpp
  ksettingsonlinequotes.cpp
  ksettingsregister.cpp
  ksettingsschedules.cpp
  ksettingsreports.cpp
  ksettingskmymoney.cpp
  ksettingsplugins.cpp
  )

set (libsettings_a_UI
  ksettingscolors.ui
  ksettingsfonts.ui
  ksettingsicons.ui
  ksettingsforecast.ui
  ksettingsgeneral.ui
  ksettingsgpg.ui
  ksettingshome.ui
  ksettingsonlinequotes.ui
  ksettingsregister.ui
  ksettingsschedules.ui
  ksettingsreports.ui
  )

ki18n_wrap_ui(libsettings_a_SOURCES ${libsettings_a_UI} )

add_library(settings STATIC ${libsettings_a_SOURCES})

# TODO: cleanup dependencies
target_link_libraries(settings PUBLIC
  KF5::Completion
  KF5::KIOWidgets
  KF5::TextWidgets
  KF5::I18n
  KF5::IconThemes
  KF5::Completion
  KF5::KCMUtils
  KF5::ItemViews
  Alkimia::alkimia
  Qt5::Sql
)

if (KF5Holidays_FOUND)
  target_link_libraries(settings PUBLIC KF5::Holidays)
endif()

add_dependencies(settings widgets kmm_config)
