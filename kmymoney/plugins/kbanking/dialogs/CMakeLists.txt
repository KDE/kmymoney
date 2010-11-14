
########### next target ###############

SET(kmm_kbanking_dialogs_la_SOURCES
    kbaccountsettings.cpp
    kbpickstartdate.cpp
    kbmapaccount.cpp
    )

SET(kmm_kbanking_dialogs_UI
    kbaccountsettings.ui
    kbpickstartdate.ui
    kbmapaccount.ui
    )

KDE4_ADD_UI_FILES(kmm_kbanking_dialogs_la_SOURCES
  ${kmm_kbanking_dialogs_UI})
KDE4_ADD_LIBRARY(kmm_kbanking_dialogs STATIC ${kmm_kbanking_dialogs_la_SOURCES})
