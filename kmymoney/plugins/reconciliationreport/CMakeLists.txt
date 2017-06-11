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
  Qt5::WebEngineWidgets
  Qt5::PrintSupport
  kmm_mymoney
  kmm_plugin
)

#to be deleted when QWebEngine 5.8 will be better spread across distros
if(KF5KHtml_FOUND)
  target_link_libraries(kmm_reconciliationreport KF5::KHtml)
endif()

install(TARGETS kmm_reconciliationreport
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney/")
