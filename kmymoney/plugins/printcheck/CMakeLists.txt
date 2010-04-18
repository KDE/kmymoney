# patch the version with the version defined in the build system
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/kmm_printcheck.desktop ${CMAKE_CURRENT_BINARY_DIR}/kmm_printcheck.desktop)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    )

SET(kmm_printcheck_PART_SRCS
    numbertowords.cpp
    printcheck.cpp
    )

KDE4_ADD_KCFG_FILES(kmm_printcheck_PART_SRCS pluginsettings.kcfgc)

KDE4_ADD_PLUGIN(kmm_printcheck ${kmm_printcheck_PART_SRCS})

TARGET_LINK_LIBRARIES(kmm_printcheck
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KHTML_LIBS}
                      kmm_mymoney kmm_plugin
                      )


INSTALL(TARGETS kmm_printcheck
        DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_printcheck.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

INSTALL(FILES kmm_printcheck.rc
        DESTINATION  ${DATA_INSTALL_DIR}/kmm_printcheck)

INSTALL(FILES check_template.html
        DESTINATION  ${DATA_INSTALL_DIR}/kmm_printcheck)

# the KCM module

SET(kcm_kmm_printcheck_PART_SRCS
    kcm_printcheck.cpp
    )

KDE4_ADD_KCFG_FILES(kcm_kmm_printcheck_PART_SRCS pluginsettings.kcfgc)

KDE4_ADD_UI_FILES(kcm_kmm_printcheck_PART_SRCS pluginsettingsdecl.ui)

KDE4_ADD_PLUGIN(kcm_kmm_printcheck ${kcm_kmm_printcheck_PART_SRCS})

TARGET_LINK_LIBRARIES(kcm_kmm_printcheck
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KHTML_LIBS}
                      )

INSTALL(TARGETS kcm_kmm_printcheck
        DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES kcm_kmm_printcheck.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
