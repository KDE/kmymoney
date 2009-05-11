INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/dialogs ${CMAKE_CURRENT_BINARY_DIR}/dialogs)

ADD_SUBDIRECTORY( dialogs )

########### next target ###############

SET(kmm_ofximport_PART_SRCS ofximporterplugin.cpp ofxpartner.cpp)

KDE4_ADD_KPART(kmm_ofximport SHARED ${kmm_ofximport_PART_SRCS})
TARGET_LINK_LIBRARIES(kmm_ofximport
  ${QT_AND_KDECORE_LIBS} kmm_mymoney kmm_ofximport_dialogs kmm_plugin ofx)

########### install files ###############

INSTALL(TARGETS kmm_ofximport 
  ${INSTALL_TARGETS_DEFAULT_ARGS}/kde4)

INSTALL(FILES kmm_ofximport.desktop
  DESTINATION ${SERVICES_INSTALL_DIR})
