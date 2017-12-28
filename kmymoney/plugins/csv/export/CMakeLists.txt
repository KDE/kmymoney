# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/csvexporter.json.in ${CMAKE_CURRENT_BINARY_DIR}/csvexporter.json @ONLY)

########### next target ###############

set(csvexporter_PART_SRCS
  csvexporter.cpp
  csvwriter.cpp
  csvexportdlg.cpp
)

ki18n_wrap_ui(csvexporter_PART_SRCS
  csvexportdlg.ui
)

add_library(csvexporter MODULE
  ${csvexporter_PART_SRCS}
)

target_link_libraries(csvexporter
  kmm_mymoney kmm_plugin
  Alkimia::alkimia
)

########### install files ###############

install(FILES csvexporter.rc
        DESTINATION "${KXMLGUI_INSTALL_DIR}/csvexporter")

install(TARGETS csvexporter
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney/")


#INSTALL(FILES csvexporterrc
#  DESTINATION ${CONFIG_INSTALL_DIR})
