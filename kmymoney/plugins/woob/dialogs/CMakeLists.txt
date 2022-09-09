
########### next target ###############

set(woob_dialogs_la_SOURCES
    mapaccountwizard.cpp
    accountsettings.cpp
    )

set(woob_dialogs_UI
    mapaccountwizard.ui
    accountsettings.ui
    )

ki18n_wrap_ui(woob_dialogs_la_SOURCES
  ${woob_dialogs_UI})
add_library(woob_dialogs STATIC ${woob_dialogs_la_SOURCES})
target_link_libraries (
  woob_dialogs
  PRIVATE
  Qt::Concurrent
  Qt::Widgets
  KF5::WidgetsAddons
  KF5::I18n
  Alkimia::alkimia
  )
