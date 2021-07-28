########### next target ###############

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/onlinejobpluginmockup.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/onlinejobpluginmockup.json @ONLY)

set(onlinejobpluginmockup_PART_SRCS
  onlinejobpluginmockup.cpp
)

kcoreaddons_add_plugin(onlinejobpluginmockup
                       SOURCES ${onlinejobpluginmockup_PART_SRCS}
                       INSTALL_NAMESPACE "kmymoney")

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(onlinejobpluginmockup
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

target_link_libraries(onlinejobpluginmockup
                      kmm_mymoney
                      kmm_plugin
)
