
########### next target ###############

SET(kmm_kbanking_views_la_SOURCES
    kbjobview.cpp
    )

SET(kmm_kbanking_views_UI
    kbjobview.ui
    )

KDE4_ADD_UI_FILES(kmm_kbanking_views_la_SOURCES
  ${kmm_kbanking_views_UI})
KDE4_ADD_LIBRARY(kmm_kbanking_views STATIC ${kmm_kbanking_views_la_SOURCES})
