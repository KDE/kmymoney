# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmm_printcheck.json.in ${CMAKE_CURRENT_BINARY_DIR}/kmm_printcheck.json @ONLY)

set(kmm_printcheck_PART_SRCS
    numbertowords.cpp
    printcheck.cpp
    )

kconfig_add_kcfg_files(kmm_printcheck_PART_SRCS pluginsettings.kcfgc)

add_library(kmm_printcheck MODULE ${kmm_printcheck_PART_SRCS})

target_link_libraries(kmm_printcheck
  KF5::KHtml
  kmm_mymoney
  kmm_plugin
)

install(TARGETS kmm_printcheck
        DESTINATION "${PLUGIN_INSTALL_DIR}/kmymoney/")

install(FILES kmm_printcheck.rc
        DESTINATION "${KXMLGUI_INSTALL_DIR}/kmm_printcheck")

install(FILES check_template.html
        DESTINATION "${DATA_INSTALL_DIR}/kmm_printcheck")

install(FILES check_template_green_linen.html
        DESTINATION "${DATA_INSTALL_DIR}/kmm_printcheck")

# the KCM module

set(kcm_kmm_printcheck_PART_SRCS
    kcm_printcheck.cpp
    )

kconfig_add_kcfg_files(kcm_kmm_printcheck_PART_SRCS pluginsettings.kcfgc)

ki18n_wrap_ui(kcm_kmm_printcheck_PART_SRCS pluginsettingsdecl.ui)

add_library(kcm_kmm_printcheck MODULE ${kcm_kmm_printcheck_PART_SRCS})

kcoreaddons_desktop_to_json(kcm_kmm_printcheck kcm_kmm_printcheck.desktop)

target_link_libraries(kcm_kmm_printcheck
                      KF5::KHtml
                      KF5::CoreAddons
                      )

install(TARGETS kcm_kmm_printcheck
        DESTINATION "${PLUGIN_INSTALL_DIR}")

install(FILES kcm_kmm_printcheck.desktop
        DESTINATION "${SERVICES_INSTALL_DIR}")
