
set (libsettings_a_SOURCES
  ksettingscolors.cpp
  ksettingsfonts.cpp
  ksettingsicons.cpp
  ksettingsgeneral.cpp
  ksettingshome.cpp
  ksettingsonlinequotes.cpp
  ksettingsregister.cpp
  ksettingsschedules.cpp
  ksettingskmymoney.cpp
  ksettingsplugins.cpp
  )

set (libsettings_a_UI
  ksettingscolors.ui
  ksettingsfonts.ui
  ksettingsicons.ui
  ksettingsgeneral.ui
  ksettingshome.ui
  ksettingsonlinequotes.ui
  ksettingsregister.ui
  ksettingsschedules.ui
  )

ki18n_wrap_ui(libsettings_a_SOURCES ${libsettings_a_UI} )

add_library(settings STATIC ${libsettings_a_SOURCES})

# TODO: cleanup dependencies
target_link_libraries(settings PUBLIC
  KF${QT_MAJOR_VERSION}::Completion
  KF${QT_MAJOR_VERSION}::KIOWidgets
  KF${QT_MAJOR_VERSION}::TextWidgets
  KF${QT_MAJOR_VERSION}::I18n
  KF${QT_MAJOR_VERSION}::Completion
  KF${QT_MAJOR_VERSION}::KCMUtils
  KF${QT_MAJOR_VERSION}::ItemViews
  Alkimia::alkimia
)

if (ENABLE_HOLIDAYS)
  target_link_libraries(settings PUBLIC KF${QT_MAJOR_VERSION}::Holidays)
endif()

add_dependencies(settings kmm_widgets kmm_settings)
