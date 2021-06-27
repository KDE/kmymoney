
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
  KF5::Completion
  KF5::KIOWidgets
  KF5::TextWidgets
  KF5::I18n
  KF5::Completion
  KF5::KCMUtils
  KF5::ItemViews
  Alkimia::alkimia
  kmm_oldregister
)

if (KMYMONEY_STABLE_BRANCH)
  target_link_libraries(settings PUBLIC KF5::IconThemes)
endif()

if (ENABLE_HOLIDAYS)
  target_link_libraries(settings PUBLIC KF5::Holidays)
endif()

add_dependencies(settings kmm_widgets kmm_settings)
