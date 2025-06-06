add_subdirectory(core)

# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/csvimporter.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/csvimporter.json @ONLY)

########### next target ###############
add_library(csvimporter_settings_static STATIC ${CMAKE_CURRENT_BINARY_DIR}/pluginsettings.cpp)
target_link_libraries(csvimporter_settings_static KF${QT_MAJOR_VERSION}::ConfigGui)
kconfig_add_kcfg_files(csvimporter_settings_static pluginsettings.kcfgc)

set(csvimporter_PART_SRCS
  csvimporter.cpp
  csvwizard.cpp
  bankingwizardpage.cpp
  investmentwizardpage.cpp
  priceswizardpage.cpp
  transactiondlg.cpp
  securitiesdlg.cpp
  securitydlg.cpp
  currenciesdlg.cpp
)

qt_add_resources(csvimporter_PART_SRCS csvimporter.qrc)

ki18n_wrap_ui(csvimporter_PART_SRCS
  csvwizard.ui
  transactiondlg.ui
  introwizardpage.ui
  separatorwizardpage.ui
  rowswizardpage.ui
  bankingwizardpage.ui
  investmentwizardpage.ui
  priceswizardpage.ui
  formatswizardpage.ui
  securitiesdlg.ui
  securitydlg.ui
  currenciesdlg.ui
)

kmymoney_add_plugin(csvimporter SOURCES ${csvimporter_PART_SRCS})

target_link_libraries(csvimporter
  PUBLIC
    csvimporter_settings_static
    kmm_csvimportercore
    kmm_plugin
    kmm_base_widgets
)

########### install files ###############

# the KCM module
set(kcm_csvimporter_PART_SRCS
    kcm_csvimporter.cpp
    )
set_property(SOURCE kcm_csvimporter.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/pluginsettings.h)

ki18n_wrap_ui(kcm_csvimporter_PART_SRCS pluginsettingsdecl.ui)

kmymoney_add_plugin_kcm(kcm_csvimporter SOURCES ${kcm_csvimporter_PART_SRCS})

target_link_libraries(kcm_csvimporter
                      csvimporter_settings_static
                      KF${QT_MAJOR_VERSION}::I18n
                      KF${QT_MAJOR_VERSION}::CoreAddons
                      KF${QT_MAJOR_VERSION}::ConfigWidgets
                      kmm_kcmodule
                      )
