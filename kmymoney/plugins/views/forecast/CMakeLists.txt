configure_file(${CMAKE_CURRENT_SOURCE_DIR}/forecastview.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/forecastview.json @ONLY)

add_library(forecast_settings_static STATIC ${CMAKE_CURRENT_BINARY_DIR}/forecastviewsettings.cpp)
target_link_libraries(forecast_settings_static KF${QT_MAJOR_VERSION}::ConfigGui)
kconfig_add_kcfg_files(forecast_settings_static forecastviewsettings.kcfgc)

set(forecastview_SOURCES
  forecastview.cpp
  kforecastview.cpp
  fixedcolumntreeview.cpp
  )
set_property(SOURCE kforecastview.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/forecastviewsettings.h)

ki18n_wrap_ui(forecastview_SOURCES kforecastview.ui)

qt_add_resources(forecastview_SOURCES forecastview.qrc)

kmymoney_add_plugin(forecastview SOURCES ${forecastview_SOURCES})

target_link_libraries(forecastview
  PUBLIC
    forecast_settings_static
    kmm_plugin
    KF${QT_MAJOR_VERSION}::TextWidgets
    kmm_widgets
    kmymoney_common
)

# the KCM module

set(kcm_forecastview_PART_SRCS
    kcm_forecastview.cpp
    )

ki18n_wrap_ui(kcm_forecastview_PART_SRCS forecastviewsettings.ui)

kmymoney_add_plugin_kcm(kcm_forecastview SOURCES ${kcm_forecastview_PART_SRCS})

target_link_libraries(kcm_forecastview
                      forecast_settings_static
                      KF${QT_MAJOR_VERSION}::I18n
                      KF${QT_MAJOR_VERSION}::ConfigWidgets
                      KF${QT_MAJOR_VERSION}::Completion
                      KF${QT_MAJOR_VERSION}::KIOWidgets
                      KF${QT_MAJOR_VERSION}::CoreAddons
                      kmm_kcmodule
                      kmm_settings
                      )
