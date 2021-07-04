# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/checkprinting.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/checkprinting.json @ONLY)

set(checkprinting_PART_SRCS
    numbertowords.cpp
    checkprinting.cpp
    )

kconfig_add_kcfg_files(checkprinting_PART_SRCS pluginsettings.kcfgc)

qt5_add_resources(checkprinting_PART_SRCS checkprinting.qrc)

kcoreaddons_add_plugin(checkprinting
                       SOURCES ${checkprinting_PART_SRCS}
                       JSON "${CMAKE_CURRENT_BINARY_DIR}/checkprinting.json"
                       INSTALL_NAMESPACE "kmymoney")

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(checkprinting
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

target_link_libraries(checkprinting
  Qt5::PrintSupport
  KF5::I18n
  kmm_mymoney
  kmm_plugin
  kmm_printer
  kmm_selections
)

if(ENABLE_WEBENGINE)
 target_link_libraries(checkprinting Qt5::WebEngineWidgets)
else(ENABLE_WEBENGINE)
 target_link_libraries(checkprinting KF5::WebKit)
endif(ENABLE_WEBENGINE)

# the KCM module

set(kcm_checkprinting_PART_SRCS
    kcm_checkprinting.cpp
    )

kconfig_add_kcfg_files(kcm_checkprinting_PART_SRCS pluginsettings.kcfgc)

ki18n_wrap_ui(kcm_checkprinting_PART_SRCS pluginsettingsdecl.ui)

kcoreaddons_add_plugin(kcm_checkprinting
                       SOURCES ${kcm_checkprinting_PART_SRCS}
                       JSON "${CMAKE_CURRENT_BINARY_DIR}/kcm_checkprinting.json"
                       INSTALL_NAMESPACE "kmymoney")

kcoreaddons_desktop_to_json(kcm_checkprinting kcm_checkprinting.desktop)

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(kcm_checkprinting
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

target_link_libraries(
                      kcm_checkprinting
                      kmm_url
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

install(FILES kcm_checkprinting.desktop
        DESTINATION "${SERVICES_INSTALL_DIR}")
