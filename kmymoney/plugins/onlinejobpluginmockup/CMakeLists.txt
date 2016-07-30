########### next target ###############

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmm_onlinejobpluginmockup.json.in ${CMAKE_CURRENT_BINARY_DIR}/kmm_onlinejobpluginmockup.json @ONLY)

set(kmm_onlinejobpluginmockup_PART_SRCS
  onlinejobpluginmockup.cpp
)

add_library(kmm_onlinejobpluginmockup MODULE ${kmm_onlinejobpluginmockup_PART_SRCS})

target_link_libraries(kmm_onlinejobpluginmockup
                      kmm_mymoney
                      kmm_plugin
)


########### install files ###############

install(TARGETS kmm_onlinejobpluginmockup
        DESTINATION "${PLUGIN_INSTALL_DIR}/kmymoney/")
