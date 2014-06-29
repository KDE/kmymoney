# patch the version with the version defined in the build system
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/kmm_csvexport.desktop
  ${CMAKE_CURRENT_BINARY_DIR}/kmm_csvexport.desktop
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

########### next target ###############

SET(kmm_csvexport_PART_SRCS
  csvwriter.cpp
  csvexporterplugin.cpp
  csvexportdlg.cpp
)

qt5_wrap_ui(kmm_csvexport_PART_SRCS
  csvexportdlg.ui
)

KDE4_ADD_PLUGIN(kmm_csvexport
  ${kmm_csvexport_PART_SRCS}
)

TARGET_LINK_LIBRARIES(kmm_csvexport
  kmm_mymoney kmm_plugin
  ${KDE4_KDEUI_LIBS} KF5::KIOWidgets
  ${GMP_LIBRARIES}
  Alkimia::alkimia #kmymoney_common kmm_config dialogs widgets models
)

########### install files ###############

INSTALL(FILES kmm_csvexport.rc
        DESTINATION  ${DATA_INSTALL_DIR}/kmm_csvexport)

INSTALL(TARGETS kmm_csvexport
        DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_csvexport.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

#INSTALL(FILES csvexporterrc
#  DESTINATION ${CONFIG_INSTALL_DIR})
