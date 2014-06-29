# patch the version with the version defined in the build system
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/kmm_printcheck.desktop ${CMAKE_CURRENT_BINARY_DIR}/kmm_printcheck.desktop)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    )

SET(kmm_printcheck_PART_SRCS
    numbertowords.cpp
    printcheck.cpp
    )

kconfig_add_kcfg_files(kmm_printcheck_PART_SRCS pluginsettings.kcfgc)

add_library(kmm_printcheck MODULE ${kmm_printcheck_PART_SRCS})

TARGET_LINK_LIBRARIES(kmm_printcheck
                      ${KDE4_KDEUI_LIBS}
                      KF5::KHtml
                      ${GMP_LIBRARIES} 
                      Alkimia::alkimia
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

INSTALL(FILES check_template_green_linen.html
        DESTINATION  ${DATA_INSTALL_DIR}/kmm_printcheck)

# the KCM module

SET(kcm_kmm_printcheck_PART_SRCS
    kcm_printcheck.cpp
    )

kconfig_add_kcfg_files(kcm_kmm_printcheck_PART_SRCS pluginsettings.kcfgc)

qt5_wrap_ui(kcm_kmm_printcheck_PART_SRCS pluginsettingsdecl.ui)

add_library(kcm_kmm_printcheck MODULE ${kcm_kmm_printcheck_PART_SRCS})

TARGET_LINK_LIBRARIES(kcm_kmm_printcheck
                      ${KDE4_KDEUI_LIBS}
                      KF5::KHtml
                      )

INSTALL(TARGETS kcm_kmm_printcheck
        DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES kcm_kmm_printcheck.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
