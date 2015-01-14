
########### next target ###############

set(kmm_kbanking_dialogs_la_SOURCES
  kbaccountsettings.cpp
  kbpickstartdate.cpp
  kbmapaccount.cpp
)

set(kmm_kbanking_dialogs_UI
  kbaccountsettings.ui
  kbpickstartdate.ui
  kbmapaccount.ui
)

qt5_wrap_ui(kmm_kbanking_dialogs_la_SOURCES

add_library(kmm_kbanking_dialogs STATIC ${kmm_kbanking_dialogs_la_SOURCES})
target_link_libraries(kmm_kbanking_dialogs KF5::KDELibs4Support)
target_link_libraries( kmm_kbanking_dialogs
  widgets
  dialogs
)
