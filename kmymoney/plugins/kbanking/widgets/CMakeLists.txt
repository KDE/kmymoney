
########### next target ###############

set(kbanking_widgets_la_SOURCES
  chiptandialog.cpp
  phototandialog.cpp
  kbaccountlist.cpp
  kbjoblist.cpp
)

set(kbanking_widgets_UI
  chiptandialog.ui
  phototandialog.ui
)

kconfig_add_kcfg_files(kbanking_widgets_la_SOURCES ../kbankingsettings.kcfgc)

ki18n_wrap_ui(kbanking_widgets_la_SOURCES ${kbanking_widgets_UI})

add_library(kbanking_widgets STATIC ${kbanking_widgets_la_SOURCES})

target_link_libraries(kbanking_widgets
  Qt5::Widgets
  Qt5::QuickWidgets
  aqbanking::aqbanking
  KF5::I18n
  KF5::ConfigCore
  KF5::ConfigGui
)
