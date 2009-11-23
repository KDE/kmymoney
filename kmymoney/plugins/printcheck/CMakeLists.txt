# patch the version with the version defined in the build system
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/kmm_printcheck.desktop ${CMAKE_CURRENT_BINARY_DIR}/kmm_printcheck.desktop)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    )

SET(printcheck_PART_SRCS
    numbertowords.cpp
    printcheck.cpp
    )

KDE4_ADD_KCFG_FILES(kmm_printcheck_PART_SRCS pluginsettings.kcfgc)

KDE4_ADD_PLUGIN(kmm_printcheck ${kmm_printcheck_PART_SRCS})

TARGET_LINK_LIBRARIES(kmm_printcheck
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KDE3SUPPORT_LIBS}
                      )


INSTALL(TARGETS kmm_printcheck
        DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_printcheck.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

INSTALL_FILES( ${DATA_INSTALL_DIR}/kmm_printcheck printcheck.rc)
INSTALL_FILES( ${DATA_INSTALL_DIR}/kmm_printcheck check_template.html)

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
                      ${KDE4_KDE3SUPPORT_LIBS}
                      )

INSTALL(TARGETS kcm_kmm_printcheck
        DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES kcm_kmm_printcheck.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
