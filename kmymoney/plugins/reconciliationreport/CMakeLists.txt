# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/reconciliationreport.json.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/reconciliationreport.json
  @ONLY
)

set(reconciliationreport_PART_SRCS
    kreconciliationreportdlg.cpp
    reconciliationreport.cpp
   )

ki18n_wrap_ui(reconciliationreport_PART_SRCS kreconciliationreportdlgdecl.ui )

kcoreaddons_add_plugin(reconciliationreport
                       SOURCES ${reconciliationreport_PART_SRCS}
                       JSON "${CMAKE_CURRENT_BINARY_DIR}/reconciliationreport.json"
                       INSTALL_NAMESPACE "kmymoney")

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(reconciliationreport
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

target_link_libraries(reconciliationreport
  Qt5::PrintSupport
  kmm_mymoney
  kmm_plugin
)

if(ENABLE_WEBENGINE)
 target_link_libraries(reconciliationreport Qt5::WebEngineWidgets)
else(ENABLE_WEBENGINE)
 target_link_libraries(reconciliationreport KF5::WebKit)
endif(ENABLE_WEBENGINE)
