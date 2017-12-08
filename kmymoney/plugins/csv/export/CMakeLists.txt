# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/csvexport.json.in ${CMAKE_CURRENT_BINARY_DIR}/csvexport.json @ONLY)

########### next target ###############

set(kmm_csvexport_PART_SRCS
  csvwriter.cpp
  csvexporterplugin.cpp
  csvexportdlg.cpp
)

ki18n_wrap_ui(kmm_csvexport_PART_SRCS
  csvexportdlg.ui
)

add_library(kmm_csvexport MODULE
  ${kmm_csvexport_PART_SRCS}
)

target_link_libraries(kmm_csvexport
  kmm_mymoney kmm_plugin
  Alkimia::alkimia
)

########### install files ###############

install(FILES kmm_csvexport.rc
        DESTINATION "${KXMLGUI_INSTALL_DIR}/kmm_csvexport")

install(TARGETS kmm_csvexport
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney/")


#INSTALL(FILES csvexporterrc
#  DESTINATION ${CONFIG_INSTALL_DIR})
