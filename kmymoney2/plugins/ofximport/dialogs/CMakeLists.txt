INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

########### next target ###############

SET(kmm_ofximport_dialogs_la_SOURCES
  kofxdirectconnectdlg.cpp konlinebankingsetupwizard.cpp
  konlinebankingstatus.cpp mymoneyofxconnector.cpp
  )

SET(kmm_ofximport_dialogs_UI
  kofxdirectconnectdlgdecl.ui konlinebankingsetupdecl.ui
  konlinebankingstatusdecl.ui
  )

KDE4_ADD_UI_FILES(kmm_ofximport_dialogs_la_SOURCES
  ${kmm_ofximport_dialogs_UI})
KDE4_ADD_LIBRARY(kmm_ofximport_dialogs STATIC ${kmm_ofximport_dialogs_la_SOURCES})
