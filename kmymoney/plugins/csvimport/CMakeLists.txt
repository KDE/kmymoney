add_subdirectory( tests )

# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmm_csvimport.desktop
  ${CMAKE_CURRENT_BINARY_DIR}/kmm_csvimport.desktop
)

########### next target ###############

set(kmm_csvimport_PART_SRCS
  csvdialog.cpp
  csvwizard.cpp
  investprocessing.cpp
  redefinedlg.cpp
  convdate.cpp
  csvutil.cpp
  symboltabledlg.cpp
)

kconfig_add_kcfg_files(kmm_csvimport_PART_SRCS pluginsettings.kcfgc)

ki18n_wrap_ui(kmm_csvimport_PART_SRCS
  csvwizard.ui
  redefinedlgdecl.ui
  introwizardpage.ui
  separatorwizardpage.ui
  rowswizardpage.ui
  bankingwizardpage.ui
  investmentwizardpage.ui
  formatswizardpage.ui
  symboltabledlg.ui
)

# Use static library for tests only
add_library(kmm_csvimport_static STATIC ${kmm_csvimport_PART_SRCS})
add_library(kmm_csvimport MODULE csvimporterplugin.cpp)

kcoreaddons_desktop_to_json(kmm_csvimport kmm_csvimport.desktop)

target_link_libraries(kmm_csvimport_static
  PUBLIC
    kmm_mymoney
    kmm_plugin
    dialogs
)

target_link_libraries(kmm_csvimport kmm_csvimport_static)

########### install files ###############

install(FILES kmm_csvimport.rc
        DESTINATION  ${KXMLGUI_INSTALL_DIR}/kmm_csvimport)

install(TARGETS kmm_csvimport
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_csvimport.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

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
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kcm_kmm_csvimport.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
