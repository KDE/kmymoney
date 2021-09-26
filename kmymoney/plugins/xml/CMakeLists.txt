add_subdirectory( libkgpgfile )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xmlstorage.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/xmlstorage.json @ONLY)

set(xmlstorage_SOURCES
  xmlstorage.cpp
  mymoneystoragexml.cpp
  mymoneystoragenames.cpp
  mymoneystorageanon.cpp
  kgpgkeyselectiondlg.cpp
  )

ki18n_wrap_ui(xmlstorage_SOURCES kgpgkeyselectiondlg.ui)

kmymoney_add_plugin(xmlstorage
                       SOURCES ${xmlstorage_SOURCES})

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(xmlstorage
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

target_link_libraries(xmlstorage
  PUBLIC
    kmm_plugin
  PRIVATE
    Qt5::Xml
    KF5::Archive
    KF5::I18n
    KF5::CoreAddons
    kgpgfile
    kmymoney_common
    kmm_mymoney
    xmlstoragehelper
    onlinetask_interfaces
)

# install(FILES kmymoney-xmlstorageplugin.desktop
#   DESTINATION ${SERVICETYPES_INSTALL_DIR}
# )

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()


# the KCM module
set(kcm_xmlstorage_PART_SRCS
    kcm_xmlstorage.cpp
    )

kconfig_add_kcfg_files(kcm_xmlstorage_PART_SRCS xmlstoragesettings.kcfgc)

ki18n_wrap_ui(kcm_xmlstorage_PART_SRCS xmlstoragesettings.ui)

kmymoney_add_plugin(kcm_xmlstorage
                       SOURCES ${kcm_xmlstorage_PART_SRCS})

kcoreaddons_desktop_to_json(kcm_xmlstorage kcm_xmlstorage.desktop)

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(kcm_xmlstorage
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

target_link_libraries(kcm_xmlstorage
  PRIVATE
    KF5::I18n
    KF5::ConfigWidgets
    KF5::Completion
    kgpgfile
    kmm_settings
)

install(FILES kcm_xmlstorage.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
