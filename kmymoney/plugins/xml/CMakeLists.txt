configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xmlstorage.json.in ${CMAKE_CURRENT_BINARY_DIR}/xmlstorage.json @ONLY)

set(xmlstorage_SOURCES
  xmlstorage.cpp
  mymoneystoragexml.cpp
  mymoneystoragenames.cpp
  mymoneystorageanon.cpp
  kgpgkeyselectiondlg.cpp
  )

ki18n_wrap_ui(xmlstorage_SOURCES kgpgkeyselectiondlg.ui)

kcoreaddons_add_plugin(xmlstorage
                       SOURCES ${xmlstorage_SOURCES}
                       JSON "${CMAKE_CURRENT_BINARY_DIR}/xmlstorage.json"
                       INSTALL_NAMESPACE "kmymoney")

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
)

# install(FILES kmymoney-xmlstorageplugin.desktop
#   DESTINATION ${SERVICETYPES_INSTALL_DIR}
# )

# if(BUILD_TESTING)
#   add_subdirectory(tests)
# endif()
