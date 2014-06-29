INCLUDE_DIRECTORIES(${LIBOFX_INCLUDE_DIR})

########### next target ###############

SET(kmm_ofximport_dialogs_la_SOURCES
  kofxdirectconnectdlg.cpp konlinebankingsetupwizard.cpp
  konlinebankingstatus.cpp mymoneyofxconnector.cpp
  )

SET(kmm_ofximport_dialogs_UI
  kofxdirectconnectdlgdecl.ui konlinebankingsetupwizard.ui
  konlinebankingstatusdecl.ui
  )

qt5_wrap_ui(kmm_ofximport_dialogs_la_SOURCES
  ${kmm_ofximport_dialogs_UI})
add_library(kmm_ofximport_dialogs STATIC ${kmm_ofximport_dialogs_la_SOURCES})
