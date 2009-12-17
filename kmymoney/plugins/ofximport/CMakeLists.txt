# patch the version with the version defined in the build system
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/kmm_ofximport.desktop ${CMAKE_CURRENT_BINARY_DIR}/kmm_ofximport.desktop)
ADD_SUBDIRECTORY( dialogs )
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/dialogs
                    ${CMAKE_CURRENT_BINARY_DIR}/dialogs
                    ${LIBOFX_INCLUDE_DIR})

########### next target ###############

SET(kmm_ofximport_PART_SRCS ofximporterplugin.cpp ofxpartner.cpp)

KDE4_ADD_PLUGIN(kmm_ofximport ${kmm_ofximport_PART_SRCS})

TARGET_LINK_LIBRARIES(kmm_ofximport
                      ${KDE4_KDEUI_LIBS} ${KDE4_KDE3SUPPORT_LIBS} kmm_mymoney kmm_ofximport_dialogs kmm_plugin ${LIBOFX_LIBRARIES})

########### install files ###############

INSTALL(FILES kmm_ofximport.rc
        DESTINATION  ${DATA_INSTALL_DIR}/kmm_ofximport)

INSTALL(TARGETS kmm_ofximport
        DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_ofximport.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
