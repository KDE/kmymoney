# patch the version with the version defined in the build system
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/kmm_reconciliationreport.desktop ${CMAKE_CURRENT_BINARY_DIR}/kmm_reconciliationreport.desktop)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    )

SET(kmm_reconciliationreport_PART_SRCS
    kreconciliationreportdlg.cpp
    reconciliationreport.cpp
   )

qt5_wrap_ui(kmm_reconciliationreport_PART_SRCS kreconciliationreportdlgdecl.ui )

add_library(kmm_reconciliationreport MODULE ${kmm_reconciliationreport_PART_SRCS})

TARGET_LINK_LIBRARIES(kmm_reconciliationreport
                      ${KDE4_KDEUI_LIBS}
                      KF5::KHtml
                      Alkimia::alkimia
                      kmm_mymoney kmm_plugin
                      )

INSTALL(TARGETS kmm_reconciliationreport
        DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_reconciliationreport.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
