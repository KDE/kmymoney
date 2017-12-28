# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/reconciliationreport.json.in
  ${CMAKE_CURRENT_BINARY_DIR}/reconciliationreport.json
  @ONLY
)

set(reconciliationreport_PART_SRCS
    kreconciliationreportdlg.cpp
    reconciliationreport.cpp
   )

ki18n_wrap_ui(reconciliationreport_PART_SRCS kreconciliationreportdlgdecl.ui )

add_library(reconciliationreport MODULE ${reconciliationreport_PART_SRCS})

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

install(TARGETS reconciliationreport
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney/")
