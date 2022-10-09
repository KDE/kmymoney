# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ofximporter.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/ofximporter.json @ONLY)

add_subdirectory( dialogs )

########### next target ###############

set(ofximporter_PART_SRCS
  ofximporter.cpp
  ofxpartner.cpp
)

set(ofximporter_PART_UI
  importoption.ui
)

ki18n_wrap_ui(ofximporter_PART_SRCS ${ofximporter_PART_UI})

qt5_add_resources(ofximporter_PART_SRCS ofximporter.qrc)

kmymoney_add_plugin(ofximporter SOURCES ${ofximporter_PART_SRCS})

target_link_libraries(ofximporter
  ofximporter_dialogs
  kmm_mymoney
  kmm_widgets
  kmm_plugin
  kmm_keychain
  Qt::Xml
  Alkimia::alkimia
  libofx::libofx
)

target_include_directories(ofximporter
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
  ${CMAKE_CURRENT_BINARY_DIR}/dialogs
)
