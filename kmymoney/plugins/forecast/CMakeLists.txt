configure_file(${CMAKE_CURRENT_SOURCE_DIR}/forecastview.json.in ${CMAKE_CURRENT_BINARY_DIR}/forecastview.json @ONLY)

set(forecastview_SOURCES
  forecastview.cpp
  kforecastview.cpp
  fixedcolumntreeview.cpp
  )

ki18n_wrap_ui(forecastview_SOURCES kforecastview.ui)
kconfig_add_kcfg_files(forecastview_SOURCES forecastviewsettings.kcfgc)

add_library(forecastview MODULE ${forecastview_SOURCES} )

target_link_libraries(forecastview
  PUBLIC
    kmm_plugin
    KF5::TextWidgets
    reports
    viewbase
)

install(TARGETS forecastview
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney")

# the KCM module

set(kcm_forecastview_PART_SRCS
    kcm_forecastview.cpp
    )

kconfig_add_kcfg_files(kcm_forecastview_PART_SRCS forecastviewsettings.kcfgc)

ki18n_wrap_ui(kcm_forecastview_PART_SRCS forecastviewsettings.ui)

add_library(kcm_forecastview MODULE ${kcm_forecastview_PART_SRCS})

kcoreaddons_desktop_to_json(kcm_forecastview kcm_forecastview.desktop)

target_link_libraries(kcm_forecastview
                      KF5::I18n
                      KF5::ConfigWidgets
                      KF5::Completion
                      KF5::KIOWidgets
                      KF5::CoreAddons
                      )

install(TARGETS kcm_forecastview
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney")

install(FILES kcm_forecastview.desktop
        DESTINATION "${SERVICES_INSTALL_DIR}")
