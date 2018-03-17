
########### next target ###############

set(weboob_dialogs_la_SOURCES
    mapaccountwizard.cpp
    accountsettings.cpp
    )

set(weboob_dialogs_UI
    mapaccountwizard.ui
    accountsettings.ui
    )

ki18n_wrap_ui(weboob_dialogs_la_SOURCES
  ${weboob_dialogs_UI})
add_library(weboob_dialogs STATIC ${weboob_dialogs_la_SOURCES})
target_link_libraries (
  weboob_dialogs
  PRIVATE
  Qt5::Concurrent
  Qt5::Widgets
  KF5::WidgetsAddons
  KF5::I18n
  Alkimia::alkimia
  )
