ADD_SUBDIRECTORY( dialogs )
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/dialogs
                    ${CMAKE_CURRENT_BINARY_DIR}/dialogs)

########### next target ###############

SET(kmm_ofximport_PART_SRCS ofximporterplugin.cpp ofxpartner.cpp)

KDE4_ADD_PLUGIN(kmm_ofximport ${kmm_ofximport_PART_SRCS})
TARGET_LINK_LIBRARIES(kmm_ofximport
                      ${KDE4_KDEUI_LIBS} ${KDE4_KDE3SUPPORT_LIBS} kmm_mymoney kmm_ofximport_dialogs kmm_plugin ofx)

########### install files ###############

INSTALL(TARGETS kmm_ofximport
  ${INSTALL_TARGETS_DEFAULT_ARGS}/kde4)

INSTALL(FILES kmm_ofximport.desktop
  DESTINATION ${SERVICES_INSTALL_DIR})
