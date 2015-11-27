
########### next target ###############

set(kmm_weboob_dialogs_la_SOURCES
    mapaccount.cpp
    webaccount.cpp
    )

set(kmm_weboob_dialogs_UI
    mapaccount.ui
    webaccount.ui
    )

kde4_add_ui_files(kmm_weboob_dialogs_la_SOURCES
  ${kmm_weboob_dialogs_UI})
kde4_add_library(kmm_weboob_dialogs STATIC ${kmm_weboob_dialogs_la_SOURCES})
