# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/printcheck.json.in ${CMAKE_CURRENT_BINARY_DIR}/printcheck.json @ONLY)

set(printcheck_PART_SRCS
    numbertowords.cpp
    printcheck.cpp
    ../../widgets/selectedtransaction.cpp
    )

kconfig_add_kcfg_files(printcheck_PART_SRCS pluginsettings.kcfgc)

add_library(printcheck MODULE ${printcheck_PART_SRCS})

target_link_libraries(printcheck
  Qt5::PrintSupport
  KF5::I18n
  kmm_mymoney
  kmm_plugin
)

if(ENABLE_WEBENGINE)
 target_link_libraries(printcheck Qt5::WebEngineWidgets)
else(ENABLE_WEBENGINE)
 target_link_libraries(printcheck KF5::WebKit)
endif(ENABLE_WEBENGINE)

install(TARGETS printcheck
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney/")

install(FILES printcheck.rc
        DESTINATION "${KXMLGUI_INSTALL_DIR}/printcheck")

install(FILES check_template.html
        DESTINATION "${DATA_INSTALL_DIR}/printcheck")

install(FILES check_template_green_linen.html
        DESTINATION "${DATA_INSTALL_DIR}/printcheck")

# the KCM module

set(kcm_printcheck_PART_SRCS
    kcm_printcheck.cpp
    )

kconfig_add_kcfg_files(kcm_printcheck_PART_SRCS pluginsettings.kcfgc)

ki18n_wrap_ui(kcm_printcheck_PART_SRCS pluginsettingsdecl.ui)

add_library(kcm_printcheck MODULE ${kcm_printcheck_PART_SRCS})

kcoreaddons_desktop_to_json(kcm_printcheck kcm_printcheck.desktop)

target_link_libraries(kcm_printcheck
                      Qt5::PrintSupport
                      KF5::I18n
                      KF5::ConfigWidgets
                      KF5::Completion
                      KF5::KIOWidgets
                      KF5::CoreAddons
                      )

if(ENABLE_WEBENGINE)
 target_link_libraries(kcm_printcheck Qt5::WebEngineWidgets)
else(ENABLE_WEBENGINE)
 target_link_libraries(kcm_printcheck KF5::WebKit)
endif(ENABLE_WEBENGINE)

install(TARGETS kcm_printcheck
        DESTINATION "${KDE_INSTALL_PLUGINDIR}")

install(FILES kcm_printcheck.desktop
        DESTINATION "${SERVICES_INSTALL_DIR}")
