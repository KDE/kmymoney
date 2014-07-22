
########### next target ###############

set(kmm_kbanking_widgets_la_SOURCES
    kbaccountlist.cpp
    kbjoblist.cpp
    )

kde4_add_library(kmm_kbanking_widgets STATIC ${kmm_kbanking_widgets_la_SOURCES})
