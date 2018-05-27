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

kcoreaddons_add_plugin(csvexporter
                       SOURCES ${csvexporter_PART_SRCS}
                       JSON "${CMAKE_CURRENT_BINARY_DIR}/csvexporter.json"
                       INSTALL_NAMESPACE "kmymoney")

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(csvexporter
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

target_link_libraries(csvexporter
  PUBLIC
    kmm_mymoney
    Alkimia::alkimia
  PRIVATE
    KF5::Completion
    KF5::KIOWidgets
    kmm_plugin
)

########### install files ###############

install(FILES csvexporter.rc
        DESTINATION "${KXMLGUI_INSTALL_DIR}/csvexporter")

#INSTALL(FILES csvexporterrc
#  DESTINATION ${CONFIG_INSTALL_DIR})
