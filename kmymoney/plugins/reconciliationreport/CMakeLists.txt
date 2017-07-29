# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmm_reconciliationreport.json.in
  ${CMAKE_CURRENT_BINARY_DIR}/kmm_reconciliationreport.json
  @ONLY
)

set(kmm_reconciliationreport_PART_SRCS
    kreconciliationreportdlg.cpp
    reconciliationreport.cpp
   )

ki18n_wrap_ui(kmm_reconciliationreport_PART_SRCS kreconciliationreportdlgdecl.ui )

add_library(kmm_reconciliationreport MODULE ${kmm_reconciliationreport_PART_SRCS})

target_link_libraries(kmm_reconciliationreport
  Qt5::PrintSupport
  kmm_mymoney
  kmm_plugin
)

if(ENABLE_WEBENGINE)
 target_link_libraries(kmm_reconciliationreport Qt5::WebEngineWidgets)
else(ENABLE_WEBENGINE)
 target_link_libraries(kmm_reconciliationreport KF5::WebKit)
endif(ENABLE_WEBENGINE)

install(TARGETS kmm_reconciliationreport
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney/")
