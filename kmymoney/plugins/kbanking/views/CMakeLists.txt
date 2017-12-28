
########### next target ###############

set(kbanking_views_la_SOURCES
    kbjobview.cpp
    )

set(kbanking_views_UI
    kbjobview.ui
    )

ki18n_wrap_ui(kbanking_views_la_SOURCES
  ${kbanking_views_UI})

add_library(kbanking_views STATIC ${kbanking_views_la_SOURCES})

target_link_libraries(kbanking_views
  Qt5::Widgets
)
