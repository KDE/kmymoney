if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/csvimport.json.in ${CMAKE_CURRENT_BINARY_DIR}/csvimport.json @ONLY)

########### next target ###############

set(kmm_csvimport_core_PART_SRCS
  csvimporter.cpp
  convdate.cpp
  csvutil.cpp
)

add_library(kmm_csvimport_core SHARED ${kmm_csvimport_core_PART_SRCS})
generate_export_header(kmm_csvimport_core)
target_link_libraries(kmm_csvimport_core
  PUBLIC
    kmm_mymoney
)

########### install files ###############

install(TARGETS kmm_csvimport_core
  ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### next target ###############

set(kmm_csvimport_PART_SRCS
  csvimporterplugin.cpp
  csvwizard.cpp
  bankingwizardpage.cpp
  investmentwizardpage.cpp
  priceswizardpage.cpp
  transactiondlg.cpp
  securitiesdlg.cpp
  securitydlg.cpp
  currenciesdlg.cpp
)

kconfig_add_kcfg_files(kmm_csvimport_PART_SRCS pluginsettings.kcfgc)

ki18n_wrap_ui(kmm_csvimport_PART_SRCS
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

# Use static library for tests and prices importer only
add_library(kmm_csvimport MODULE ${kmm_csvimport_PART_SRCS})

target_link_libraries(kmm_csvimport
  PUBLIC
    kmm_csvimport_core
    kmm_plugin
)

########### install files ###############

install(FILES kmm_csvimport.rc
        DESTINATION "${KXMLGUI_INSTALL_DIR}/kmm_csvimport")

install(TARGETS kmm_csvimport
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney/")

# the KCM module
set(kcm_kmm_csvimport_PART_SRCS
    kcm_csvimport.cpp
    )

kconfig_add_kcfg_files(kcm_kmm_csvimport_PART_SRCS pluginsettings.kcfgc)

ki18n_wrap_ui(kcm_kmm_csvimport_PART_SRCS pluginsettingsdecl.ui)

add_library(kcm_kmm_csvimport MODULE ${kcm_kmm_csvimport_PART_SRCS})

kcoreaddons_desktop_to_json(kcm_kmm_csvimport kcm_kmm_csvimport.desktop)

target_link_libraries(kcm_kmm_csvimport
                      KF5::I18n
                      KF5::ConfigWidgets
                      )

install(TARGETS kcm_kmm_csvimport
        DESTINATION ${KDE_INSTALL_PLUGINDIR})

install(FILES kcm_kmm_csvimport.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
