########### next target ###############

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/onlinejobpluginmockup.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/onlinejobpluginmockup.json @ONLY)

set(onlinejobpluginmockup_PART_SRCS
  onlinejobpluginmockup.cpp
)

kmymoney_add_plugin(onlinejobpluginmockup SOURCES ${onlinejobpluginmockup_PART_SRCS})

target_link_libraries(onlinejobpluginmockup
                      kmm_mymoney
                      kmm_plugin
)
