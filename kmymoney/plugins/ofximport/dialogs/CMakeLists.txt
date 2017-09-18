include_directories(${LIBOFX_INCLUDE_DIR})

if(${LIBOFX_HAVE_CLIENTUID})
  add_definitions(-DLIBOFX_HAVE_CLIENTUID)
endif(${LIBOFX_HAVE_CLIENTUID})

########### next target ###############

set(kmm_ofximport_dialogs_la_SOURCES
  kofxdirectconnectdlg.cpp konlinebankingsetupwizard.cpp
  konlinebankingstatus.cpp mymoneyofxconnector.cpp
  )

set(kmm_ofximport_dialogs_UI
  kofxdirectconnectdlgdecl.ui konlinebankingsetupwizard.ui
  konlinebankingstatusdecl.ui
  )

ki18n_wrap_ui(kmm_ofximport_dialogs_la_SOURCES
  ${kmm_ofximport_dialogs_UI})
add_library(kmm_ofximport_dialogs STATIC ${kmm_ofximport_dialogs_la_SOURCES})
target_link_libraries(kmm_ofximport_dialogs kmm_widgets KF5::WidgetsAddons KF5::I18n KF5::CoreAddons KF5::KIOCore KF5::KIOWidgets KF5::Wallet Qt5::Widgets Qt5::Sql Qt5::Xml)
