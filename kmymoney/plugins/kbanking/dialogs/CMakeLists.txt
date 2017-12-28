
########### next target ###############

set(kbanking_dialogs_la_SOURCES
  kbaccountsettings.cpp
  kbpickstartdate.cpp
  kbmapaccount.cpp
)

set(kbanking_dialogs_UI
  kbaccountsettings.ui
  kbpickstartdate.ui
  kbmapaccount.ui
)

ki18n_wrap_ui(kbanking_dialogs_la_SOURCES ${kbanking_dialogs_UI})

add_library(kbanking_dialogs STATIC ${kbanking_dialogs_la_SOURCES})
target_link_libraries(kbanking_dialogs
  widgets
  dialogs
  aqbanking::aqbanking
)

