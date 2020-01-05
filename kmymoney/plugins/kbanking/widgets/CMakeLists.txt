
########### next target ###############

set(kmm_kbanking_widgets_la_SOURCES
  chiptandialog.cpp
  kbaccountlist.cpp
  kbjoblist.cpp
  phototandialog.cpp
)

kde4_add_ui_files(kmm_kbanking_widgets_la_SOURCES
  chiptandialog.ui
  phototandialog.ui
)

kde4_add_library(kmm_kbanking_widgets STATIC ${kmm_kbanking_widgets_la_SOURCES})
