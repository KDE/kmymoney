# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/qifexporter.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/qifexporter.json @ONLY)

########### next target ###############

set(qifexporter_PART_SRCS
  qifexporter.cpp
  mymoneyqifwriter.cpp
  kexportdlg.cpp
)

set(qifexporter_PART_UI
  kexportdlgdecl.ui
)

ki18n_wrap_ui(qifexporter_PART_SRCS ${qifexporter_PART_UI})

qt5_add_resources(qifexporter_PART_SRCS qifexporter.qrc)

kmymoney_add_plugin(qifexporter SOURCES ${qifexporter_PART_SRCS})

target_link_libraries(qifexporter
  kmm_plugin
  kmm_models
  kmm_base_widgets
  kmm_widgets
)
