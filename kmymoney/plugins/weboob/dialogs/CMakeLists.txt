
########### next target ###############

set(weboob_dialogs_la_SOURCES
    mapaccount.cpp
    webaccount.cpp
    )

set(weboob_dialogs_UI
    mapaccount.ui
    webaccount.ui
    )

ki18n_wrap_ui(weboob_dialogs_la_SOURCES
  ${weboob_dialogs_UI})
add_library(weboob_dialogs STATIC ${weboob_dialogs_la_SOURCES})
target_link_libraries(weboob_dialogs Qt5::Concurrent KF5::KrossCore widgets dialogs)
