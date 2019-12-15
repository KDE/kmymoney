# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmm_printcheck.desktop ${CMAKE_CURRENT_BINARY_DIR}/kmm_printcheck.desktop)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    )

set(kmm_printcheck_PART_SRCS
    numbertowords.cpp
    printcheck.cpp
    )

kde4_add_kcfg_files(kmm_printcheck_PART_SRCS pluginsettings.kcfgc)

kde4_add_plugin(kmm_printcheck ${kmm_printcheck_PART_SRCS})

target_link_libraries(kmm_printcheck
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KHTML_LIBS}
                      ${LIBALKIMIA_LIBRARIES}
                      kmm_mymoney kmm_plugin
                      )


install(TARGETS kmm_printcheck
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_printcheck.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES kmm_printcheck.rc
        DESTINATION  ${DATA_INSTALL_DIR}/kmm_printcheck)

install(FILES check_template.html
        DESTINATION  ${DATA_INSTALL_DIR}/kmm_printcheck)

install(FILES check_template_green_linen.html
        DESTINATION  ${DATA_INSTALL_DIR}/kmm_printcheck)

install(FILES check_all_fields.html
            DESTINATION  ${DATA_INSTALL_DIR}/kmm_printcheck)

# the KCM module

set(kcm_kmm_printcheck_PART_SRCS
    kcm_printcheck.cpp
    )

kde4_add_kcfg_files(kcm_kmm_printcheck_PART_SRCS pluginsettings.kcfgc)

kde4_add_ui_files(kcm_kmm_printcheck_PART_SRCS pluginsettingsdecl.ui)

kde4_add_plugin(kcm_kmm_printcheck ${kcm_kmm_printcheck_PART_SRCS})

target_link_libraries(kcm_kmm_printcheck
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KHTML_LIBS}
                      )

install(TARGETS kcm_kmm_printcheck
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kcm_kmm_printcheck.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
