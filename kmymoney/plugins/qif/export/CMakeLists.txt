# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/qifexporter.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/qifexporter.json @ONLY)

########### next target ###############

set(qifexporter_PART_SRCS
  qifexporter.cpp
  ../config/mymoneyqifprofile.cpp
  mymoneyqifwriter.cpp
  kexportdlg.cpp
)

set(qifexporter_PART_UI
  kexportdlgdecl.ui
)

ki18n_wrap_ui(qifexporter_PART_SRCS ${qifexporter_PART_UI})

qt5_add_resources(qifexporter_PART_SRCS qifexporter.qrc)

kcoreaddons_add_plugin(qifexporter
                       SOURCES ${qifexporter_PART_SRCS}
                       JSON "${CMAKE_CURRENT_BINARY_DIR}/qifexporter.json"
                       INSTALL_NAMESPACE "kmymoney")

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(qifexporter
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

target_link_libraries(qifexporter
  kmm_plugin
  kmm_models
  kmm_base_widgets
  kmm_widgets
)
