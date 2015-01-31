
########### next target ###############

set(kmm_kbanking_widgets_la_SOURCES
  chiptandialog.cpp
  kbaccountlist.cpp
  kbjoblist.cpp
)

set(kmm_kbanking_widgets_UI
  chiptandialog.ui
)

qt5_wrap_ui(kmm_kbanking_widgets_la_SOURCES ${kmm_kbanking_widgets_UI})

add_library(kmm_kbanking_widgets STATIC ${kmm_kbanking_widgets_la_SOURCES})
target_link_libraries(kmm_kbanking_widgets KF5::KDELibs4Support Qt5::QuickWidgets)
