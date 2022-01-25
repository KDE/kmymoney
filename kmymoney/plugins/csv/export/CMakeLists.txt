# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/csvexporter.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/csvexporter.json @ONLY)

########### next target ###############

set(csvexporter_PART_SRCS
  csvexporter.cpp
  csvwriter.cpp
  csvexportdlg.cpp
)

qt5_add_resources(csvexporter_PART_SRCS csvexporter.qrc)

ki18n_wrap_ui(csvexporter_PART_SRCS
  csvexportdlg.ui
)

kmymoney_add_plugin(csvexporter SOURCES ${csvexporter_PART_SRCS})

target_link_libraries(csvexporter
  PUBLIC
    kmm_mymoney
    Alkimia::alkimia
  PRIVATE
    KF5::Completion
    KF5::KIOWidgets
    kmm_plugin
    kmm_base_widgets
)
