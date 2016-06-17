
########### next target ###############

set(kmm_weboob_dialogs_la_SOURCES
    mapaccount.cpp
    webaccount.cpp
    )

set(kmm_weboob_dialogs_UI
    mapaccount.ui
    webaccount.ui
    )

ki18n_wrap_ui(kmm_weboob_dialogs_la_SOURCES
  ${kmm_weboob_dialogs_UI})
add_library(kmm_weboob_dialogs STATIC ${kmm_weboob_dialogs_la_SOURCES})
target_link_libraries(kmm_weboob_dialogs Qt5::Concurrent KF5::KrossCore widgets dialogs)
