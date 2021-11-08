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

kmymoney_add_plugin(reconciliationreport SOURCES ${reconciliationreport_PART_SRCS})

target_link_libraries(reconciliationreport
  Qt5::PrintSupport
  kmm_mymoney
  kmymoney_common
  kmm_plugin
  kmm_printer
)
