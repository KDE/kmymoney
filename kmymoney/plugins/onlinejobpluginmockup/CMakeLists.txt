########### next target ###############

# This is quite independet from KMyMoney, so use own versions
set(VERSION_MAJOR "1")
set(VERSION_MINOR "0")
set(VERSION_PATCH "0")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmm_onlinejobpluginmockup.desktop ${CMAKE_CURRENT_BINARY_DIR}/kmm_onlinejobpluginmockup.desktop)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

set(kmm_onlinejobpluginmockup_PART_SRCS
  onlinejobpluginmockup.cpp
)

kde4_add_plugin(kmm_onlinejobpluginmockup ${kmm_onlinejobpluginmockup_PART_SRCS})

target_link_libraries(kmm_onlinejobpluginmockup
                      kmm_mymoney
                      kmm_plugin
                      ${KDE4_KDEUI_LIBS}
                      ${LIBALKIMIA_LIBRARIES}
)


########### install files ###############

install(TARGETS kmm_onlinejobpluginmockup
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_onlinejobpluginmockup.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
