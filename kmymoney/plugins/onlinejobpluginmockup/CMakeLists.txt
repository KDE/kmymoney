########### next target ###############

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/onlinejobpluginmockup.json.in ${CMAKE_CURRENT_BINARY_DIR}/onlinejobpluginmockup.json @ONLY)

set(onlinejobpluginmockup_PART_SRCS
  onlinejobpluginmockup.cpp
)

add_library(onlinejobpluginmockup MODULE ${onlinejobpluginmockup_PART_SRCS})

target_link_libraries(onlinejobpluginmockup
                      kmm_mymoney
                      kmm_plugin
)


########### install files ###############

install(TARGETS onlinejobpluginmockup
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney/")
