# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmm_reconciliationreport.desktop ${CMAKE_CURRENT_BINARY_DIR}/kmm_reconciliationreport.desktop)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    )

set(kmm_reconciliationreport_PART_SRCS
    kreconciliationreportdlg.cpp
    reconciliationreport.cpp
   )

qt5_wrap_ui(kmm_reconciliationreport_PART_SRCS kreconciliationreportdlgdecl.ui )

add_library(kmm_reconciliationreport MODULE ${kmm_reconciliationreport_PART_SRCS})

target_link_libraries(kmm_reconciliationreport
                      ${KDE4_KDEUI_LIBS}
                      KF5::KHtml
                      Alkimia::alkimia
                      kmm_mymoney kmm_plugin
                      )

install(TARGETS kmm_reconciliationreport
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_reconciliationreport.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
