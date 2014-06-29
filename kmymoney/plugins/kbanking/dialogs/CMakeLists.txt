
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

qt5_wrap_ui(kmm_kbanking_dialogs_la_SOURCES
  ${kmm_kbanking_dialogs_UI})
add_library(kmm_kbanking_dialogs STATIC ${kmm_kbanking_dialogs_la_SOURCES})
