configure_file(${CMAKE_CURRENT_SOURCE_DIR}/forecastview.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/forecastview.json @ONLY)

add_library(forecast_settings_static STATIC ${CMAKE_CURRENT_BINARY_DIR}/forecastviewsettings.cpp)
target_link_libraries(forecast_settings_static KF5::ConfigGui)
kconfig_add_kcfg_files(forecast_settings_static forecastviewsettings.kcfgc)

set(forecastview_SOURCES
  forecastview.cpp
  kforecastview.cpp
  fixedcolumntreeview.cpp
  )
set_property(SOURCE kforecastview.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/forecastviewsettings.h)

ki18n_wrap_ui(forecastview_SOURCES kforecastview.ui)

qt5_add_resources(forecastview_SOURCES forecastview.qrc)

kmymoney_add_plugin(forecastview SOURCES ${forecastview_SOURCES})

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

kmymoney_add_plugin_kcm(kcm_forecastview SOURCES ${kcm_forecastview_PART_SRCS})

kcoreaddons_desktop_to_json(kcm_forecastview kcm_forecastview.desktop)

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
