
########### next target ###############

SET(kmm_weboob_dialogs_la_SOURCES
    mapaccount.cpp
    webaccount.cpp
    )

SET(kmm_weboob_dialogs_UI
    mapaccount.ui
    webaccount.ui
    )

KDE4_ADD_UI_FILES(kmm_weboob_dialogs_la_SOURCES
  ${kmm_weboob_dialogs_UI})
KDE4_ADD_LIBRARY(kmm_weboob_dialogs STATIC ${kmm_weboob_dialogs_la_SOURCES})
