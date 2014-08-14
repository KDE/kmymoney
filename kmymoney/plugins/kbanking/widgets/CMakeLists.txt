
########### next target ###############

set(kmm_kbanking_widgets_la_SOURCES
    kbaccountlist.cpp
    kbjoblist.cpp
    )

add_library(kmm_kbanking_widgets STATIC ${kmm_kbanking_widgets_la_SOURCES})
target_link_libraries(kmm_kbanking_widgets KF5::KDELibs4Support)
