
########### next target ###############

set(kmm_kbanking_views_la_SOURCES
  kbjobview.cpp
)

set(kmm_kbanking_views_UI
  kbjobview.ui
)

kde4_add_ui_files(kmm_kbanking_views_la_SOURCES ${kmm_kbanking_views_UI})
kde4_add_library(kmm_kbanking_views STATIC ${kmm_kbanking_views_la_SOURCES})

target_link_libraries( kmm_kbanking_views
  kmm_kbanking_dialogs
)
