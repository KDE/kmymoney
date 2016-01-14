# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmm_csvexport.desktop
  ${CMAKE_CURRENT_BINARY_DIR}/kmm_csvexport.desktop
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

########### next target ###############

set(kmm_csvexport_PART_SRCS
  csvwriter.cpp
  csvexporterplugin.cpp
  csvexportdlg.cpp
)

qt5_wrap_ui(kmm_csvexport_PART_SRCS
  csvexportdlg.ui
)

add_library(kmm_csvexport MODULE
  ${kmm_csvexport_PART_SRCS}
)

kcoreaddons_desktop_to_json(kmm_csvexport kmm_csvexport.desktop)

target_link_libraries(kmm_csvexport
  kmm_mymoney kmm_plugin
  KF5::KDELibs4Support
  Alkimia::alkimia
)

########### install files ###############

install(FILES kmm_csvexport.rc
        DESTINATION  ${KXMLGUI_INSTALL_DIR}/kmm_csvexport)

install(TARGETS kmm_csvexport
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_csvexport.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

#INSTALL(FILES csvexporterrc
#  DESTINATION ${CONFIG_INSTALL_DIR})
