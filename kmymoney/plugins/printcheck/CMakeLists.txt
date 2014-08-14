# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmm_printcheck.desktop ${CMAKE_CURRENT_BINARY_DIR}/kmm_printcheck.desktop)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    )

set(kmm_printcheck_PART_SRCS
    numbertowords.cpp
    printcheck.cpp
    )

kconfig_add_kcfg_files(kmm_printcheck_PART_SRCS pluginsettings.kcfgc)

add_library(kmm_printcheck MODULE ${kmm_printcheck_PART_SRCS})

target_link_libraries(kmm_printcheck
                      KF5::KDELibs4Support
                      KF5::KHtml
                      Alkimia::alkimia
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

# the KCM module

set(kcm_kmm_printcheck_PART_SRCS
    kcm_printcheck.cpp
    )

kconfig_add_kcfg_files(kcm_kmm_printcheck_PART_SRCS pluginsettings.kcfgc)

qt5_wrap_ui(kcm_kmm_printcheck_PART_SRCS pluginsettingsdecl.ui)

add_library(kcm_kmm_printcheck MODULE ${kcm_kmm_printcheck_PART_SRCS})

target_link_libraries(kcm_kmm_printcheck
                      KF5::KDELibs4Support
                      KF5::KHtml
                      Alkimia::alkimia
                      )

install(TARGETS kcm_kmm_printcheck
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kcm_kmm_printcheck.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
