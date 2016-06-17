add_subdirectory( tests )

# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmm_csvimport.desktop
  ${CMAKE_CURRENT_BINARY_DIR}/kmm_csvimport.desktop
)

########### next target ###############

set(kmm_csvimport_PART_SRCS
  csvdialog.cpp
  csvwizard.cpp
  investmentdlg.cpp
  investprocessing.cpp
  redefinedlg.cpp
  convdate.cpp
  csvutil.cpp
  symboltabledlg.cpp
)

ki18n_wrap_ui(kmm_csvimport_PART_SRCS
  csvdialog.ui
  csvwizard.ui
  redefinedlgdecl.ui
  introwizardpage.ui
  separatorwizardpage.ui
  bankingwizardpage.ui
  investmentwizardpage.ui
  lines-datewizardpage.ui
  completionwizardpage.ui
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

install(FILES csvimporterrc
  DESTINATION ${CONFIG_INSTALL_DIR})
