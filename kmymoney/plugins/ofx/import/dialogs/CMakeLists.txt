########### next target ###############

set(ofximporter_dialogs_la_SOURCES
  kofxdirectconnectdlg.cpp konlinebankingsetupwizard.cpp
  konlinebankingstatus.cpp mymoneyofxconnector.cpp
  )

set(ofximporter_dialogs_UI
  kofxdirectconnectdlgdecl.ui konlinebankingsetupwizard.ui
  konlinebankingstatusdecl.ui
  )

ki18n_wrap_ui(ofximporter_dialogs_la_SOURCES
  ${ofximporter_dialogs_UI})
add_library(ofximporter_dialogs STATIC ${ofximporter_dialogs_la_SOURCES})
target_link_libraries(
        ofximporter_dialogs
        kmm_widgets
        libofx::libofx
        KF5::WidgetsAddons
        KF5::I18n
        KF5::CoreAddons
        KF5::KIOCore
        KF5::KIOWidgets
        Qt::Widgets
        Qt::Xml
        kmm_keychain
)
