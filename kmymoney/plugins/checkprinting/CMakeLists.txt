# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/checkprinting.json.in ${CMAKE_CURRENT_BINARY_DIR}/checkprinting.json @ONLY)

set(checkprinting_PART_SRCS
    numbertowords.cpp
    checkprinting.cpp
    ../../widgets/selectedtransaction.cpp
    )

kconfig_add_kcfg_files(checkprinting_PART_SRCS pluginsettings.kcfgc)

add_library(checkprinting MODULE ${checkprinting_PART_SRCS})

target_link_libraries(checkprinting
  Qt5::PrintSupport
  KF5::I18n
  kmm_mymoney
  kmm_plugin
)

if(ENABLE_WEBENGINE)
 target_link_libraries(checkprinting Qt5::WebEngineWidgets)
else(ENABLE_WEBENGINE)
 target_link_libraries(checkprinting KF5::WebKit)
endif(ENABLE_WEBENGINE)

install(TARGETS checkprinting
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney/")

install(FILES checkprinting.rc
        DESTINATION "${KXMLGUI_INSTALL_DIR}/checkprinting")

install(FILES check_template.html
        DESTINATION "${DATA_INSTALL_DIR}/checkprinting")

install(FILES check_template_green_linen.html
        DESTINATION "${DATA_INSTALL_DIR}/checkprinting")

# the KCM module

set(kcm_checkprinting_PART_SRCS
    kcm_checkprinting.cpp
    )

kconfig_add_kcfg_files(kcm_checkprinting_PART_SRCS pluginsettings.kcfgc)

ki18n_wrap_ui(kcm_checkprinting_PART_SRCS pluginsettingsdecl.ui)

add_library(kcm_checkprinting MODULE ${kcm_checkprinting_PART_SRCS})

kcoreaddons_desktop_to_json(kcm_checkprinting kcm_checkprinting.desktop)

target_link_libraries(kcm_checkprinting
                      Qt5::PrintSupport
                      KF5::I18n
                      KF5::ConfigWidgets
                      KF5::Completion
                      KF5::KIOWidgets
                      KF5::CoreAddons
                      )

if(ENABLE_WEBENGINE)
 target_link_libraries(kcm_checkprinting Qt5::WebEngineWidgets)
else(ENABLE_WEBENGINE)
 target_link_libraries(kcm_checkprinting KF5::WebKit)
endif(ENABLE_WEBENGINE)

install(TARGETS kcm_checkprinting
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney")

install(FILES kcm_checkprinting.desktop
        DESTINATION "${SERVICES_INSTALL_DIR}")
