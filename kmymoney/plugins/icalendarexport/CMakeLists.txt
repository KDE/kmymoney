# patch the version with the version defined in the build system
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/kmm_icalendarexport.desktop ${CMAKE_CURRENT_BINARY_DIR}/kmm_icalendarexport.desktop)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${LIBICAL_INCLUDE_DIRS}
                    )

SET(kmm_icalendarexport_PART_SRCS
    schedulestoicalendar.cpp
    icalendarexport.cpp
    )

KDE4_ADD_KCFG_FILES(kmm_icalendarexport_PART_SRCS pluginsettings.kcfgc)

KDE4_ADD_PLUGIN(kmm_icalendarexport ${kmm_icalendarexport_PART_SRCS})

TARGET_LINK_LIBRARIES(kmm_icalendarexport
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KDE3SUPPORT_LIBS}
                      kmm_mymoney kmm_plugin
                      ${LIBICAL_LIBRARIES}
                      )

INSTALL(FILES kmm_icalendarexport.rc
        DESTINATION  ${DATA_INSTALL_DIR}/kmm_icalendarexport)

INSTALL(TARGETS kmm_icalendarexport
        DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_icalendarexport.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

# the KCM module

SET(kcm_kmm_icalendarexport_PART_SRCS
    kcm_icalendarexport.cpp
    )

KDE4_ADD_KCFG_FILES(kcm_kmm_icalendarexport_PART_SRCS pluginsettings.kcfgc)

KDE4_ADD_UI_FILES(kcm_kmm_icalendarexport_PART_SRCS pluginsettingsdecl.ui)

KDE4_ADD_PLUGIN(kcm_kmm_icalendarexport ${kcm_kmm_icalendarexport_PART_SRCS})

TARGET_LINK_LIBRARIES(kcm_kmm_icalendarexport
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KDE3SUPPORT_LIBS}
                      )

INSTALL(TARGETS kcm_kmm_icalendarexport
        DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES kcm_kmm_icalendarexport.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
