# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmm_weboob.desktop ${CMAKE_CURRENT_BINARY_DIR}/kmm_weboob.desktop)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
                    ${CMAKE_CURRENT_BINARY_DIR}/dialogs
                    ${KROSS_INCLUDES})

add_subdirectory( dialogs )

########### next target ###############

set(kmm_weboob_PART_SRCS
    weboob.cpp
    plugin.cpp
    )

kde4_add_plugin(kmm_weboob ${kmm_weboob_PART_SRCS})

target_link_libraries(kmm_weboob
                      kmm_weboob_dialogs
                      kmm_mymoney
                      kmm_widgets
                      kmm_plugin
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KROSSCORE_LIBS}
                      ${LIBALKIMIA_LIBRARY}
                      )

########### install files ###############

install(FILES weboob.py kmm_weboob.rc
        DESTINATION  ${DATA_INSTALL_DIR}/kmm_weboob)

install(TARGETS kmm_weboob
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_weboob.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
