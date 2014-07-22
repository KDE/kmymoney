# patch the version with the version defined in the build system
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/kmm_reconciliationreport.desktop ${CMAKE_CURRENT_BINARY_DIR}/kmm_reconciliationreport.desktop)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    )

SET(kmm_reconciliationreport_PART_SRCS
    kreconciliationreportdlg.cpp
    reconciliationreport.cpp
   )

KDE4_ADD_UI_FILES(kmm_reconciliationreport_PART_SRCS kreconciliationreportdlgdecl.ui )

KDE4_ADD_PLUGIN(kmm_reconciliationreport ${kmm_reconciliationreport_PART_SRCS})

TARGET_LINK_LIBRARIES(kmm_reconciliationreport
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KHTML_LIBS}
                      ${LIBALKIMIA_LIBRARY}
                      kmm_mymoney kmm_plugin
                      )

INSTALL(TARGETS kmm_reconciliationreport
        DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_reconciliationreport.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
