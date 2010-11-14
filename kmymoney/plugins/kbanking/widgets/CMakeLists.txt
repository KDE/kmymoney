
########### next target ###############

SET(kmm_kbanking_widgets_la_SOURCES
    kbaccountlist.cpp
    kbjoblist.cpp
    )

KDE4_ADD_LIBRARY(kmm_kbanking_widgets STATIC ${kmm_kbanking_widgets_la_SOURCES})
