add_subdirectory(core)

if(NOT ENABLE_CSVIMPORTER)
  return()
endif()

# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/csvimporter.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/csvimporter.json @ONLY)

########### next target ###############

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

kconfig_add_kcfg_files(csvimporter_PART_SRCS pluginsettings.kcfgc)

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

kcoreaddons_add_plugin(csvimporter
                       SOURCES ${csvimporter_PART_SRCS}
                       JSON "${CMAKE_CURRENT_BINARY_DIR}/csvimporter.json"
                       INSTALL_NAMESPACE "kmymoney")


#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(csvimporter
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

target_link_libraries(csvimporter
  PUBLIC
    kmm_csvimportercore
    kmm_plugin
)

########### install files ###############

install(FILES csvimporter.rc
        DESTINATION "${KXMLGUI_INSTALL_DIR}/csvimporter")

# the KCM module
set(kcm_csvimporter_PART_SRCS
    kcm_csvimporter.cpp
    )

kconfig_add_kcfg_files(kcm_csvimporter_PART_SRCS pluginsettings.kcfgc)

ki18n_wrap_ui(kcm_csvimporter_PART_SRCS pluginsettingsdecl.ui)

kcoreaddons_add_plugin(kcm_csvimporter
                       SOURCES ${kcm_csvimporter_PART_SRCS}
                       JSON "${CMAKE_CURRENT_BINARY_DIR}/kcm_csvimporter.json"
                       INSTALL_NAMESPACE "kmymoney")

kcoreaddons_desktop_to_json(kcm_csvimporter kcm_csvimporter.desktop)

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(kcm_csvimporter
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

target_link_libraries(kcm_csvimporter
                      KF5::I18n
                      KF5::CoreAddons
                      KF5::ConfigWidgets
                      )

install(FILES kcm_csvimporter.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
