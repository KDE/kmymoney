# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmm_icalendarexport.desktop ${CMAKE_CURRENT_BINARY_DIR}/kmm_icalendarexport.desktop)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${LIBICAL_INCLUDE_DIRS}
                    )

set(kmm_icalendarexport_PART_SRCS
    schedulestoicalendar.cpp
    icalendarexport.cpp
    )

kde4_add_kcfg_files(kmm_icalendarexport_PART_SRCS pluginsettings.kcfgc)

kde4_add_plugin(kmm_icalendarexport ${kmm_icalendarexport_PART_SRCS})

target_link_libraries(kmm_icalendarexport
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${LIBALKIMIA_LIBRARIES}
                      kmm_mymoney kmm_plugin
                      ${LIBICAL_LIBRARIES}
                      )

install(FILES kmm_icalendarexport.rc
        DESTINATION  ${DATA_INSTALL_DIR}/kmm_icalendarexport)

install(TARGETS kmm_icalendarexport
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_icalendarexport.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

# the KCM module

set(kcm_kmm_icalendarexport_PART_SRCS
    kcm_icalendarexport.cpp
    )

kde4_add_kcfg_files(kcm_kmm_icalendarexport_PART_SRCS pluginsettings.kcfgc)

kde4_add_ui_files(kcm_kmm_icalendarexport_PART_SRCS pluginsettingsdecl.ui)

kde4_add_plugin(kcm_kmm_icalendarexport ${kcm_kmm_icalendarexport_PART_SRCS})

target_link_libraries(kcm_kmm_icalendarexport
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KIO_LIBS}
                      )

install(TARGETS kcm_kmm_icalendarexport
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kcm_kmm_icalendarexport.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
