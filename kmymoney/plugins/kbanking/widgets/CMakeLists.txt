
########### next target ###############

set(kmm_kbanking_widgets_la_SOURCES
  chiptandialog.cpp
  kbaccountlist.cpp
  kbjoblist.cpp
)

set(kmm_kbanking_widgets_UI
  chiptandialog.ui
)

kconfig_add_kcfg_files(kmm_kbanking_widgets_la_SOURCES ../kbankingsettings.kcfgc)

ki18n_wrap_ui(kmm_kbanking_widgets_la_SOURCES ${kmm_kbanking_widgets_UI})

add_library(kmm_kbanking_widgets STATIC ${kmm_kbanking_widgets_la_SOURCES})

target_link_libraries(kmm_kbanking_widgets
  Qt5::Widgets
  Qt5::QuickWidgets
  aqbanking::aqbanking
  KF5::I18n
  KF5::ConfigCore
  KF5::ConfigGui
)
