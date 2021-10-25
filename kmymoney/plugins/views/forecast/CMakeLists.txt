configure_file(${CMAKE_CURRENT_SOURCE_DIR}/forecastview.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/forecastview.json @ONLY)

add_library(forecast_settings_static STATIC)
target_link_libraries(forecast_settings_static KF5::ConfigGui)
kconfig_add_kcfg_files(forecast_settings_static forecastviewsettings.kcfgc)

set(forecastview_SOURCES
  forecastview.cpp
  kforecastview.cpp
  fixedcolumntreeview.cpp
  )

ki18n_wrap_ui(forecastview_SOURCES kforecastview.ui)

qt5_add_resources(forecastview_SOURCES forecastview.qrc)

kcoreaddons_add_plugin(forecastview
                       SOURCES ${forecastview_SOURCES}
                       INSTALL_NAMESPACE "kmymoney")

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(forecastview
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

target_link_libraries(forecastview
  PUBLIC
    forecast_settings_static
    kmm_plugin
    KF5::TextWidgets
    kmm_widgets
    kmymoney_common
)

# the KCM module

set(kcm_forecastview_PART_SRCS
    kcm_forecastview.cpp
    )

ki18n_wrap_ui(kcm_forecastview_PART_SRCS forecastviewsettings.ui)

kcoreaddons_add_plugin(kcm_forecastview
                       SOURCES ${kcm_forecastview_PART_SRCS}
                       INSTALL_NAMESPACE "kmymoney")

kcoreaddons_desktop_to_json(kcm_forecastview kcm_forecastview.desktop)

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(kcm_forecastview
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

target_link_libraries(kcm_forecastview
                      forecast_settings_static
                      KF5::I18n
                      KF5::ConfigWidgets
                      KF5::Completion
                      KF5::KIOWidgets
                      KF5::CoreAddons
                      kmm_settings
                      )

install(FILES kcm_forecastview.desktop
        DESTINATION "${SERVICES_INSTALL_DIR}")
