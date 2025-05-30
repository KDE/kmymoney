# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/qifimporter.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/qifimporter.json @ONLY)

########### next target ###############

set(qifimporter_PART_SRCS
  qifimporter.cpp
  mymoneyqifreader.cpp
  kimportdlg.cpp
)

set(qifimporter_PART_UI
  kimportdlgdecl.ui
)

ki18n_wrap_ui(qifimporter_PART_SRCS ${qifimporter_PART_UI})

qt_add_resources(qifimporter_PART_SRCS qifimporter.qrc)

kmymoney_add_plugin(qifimporter SOURCES ${qifimporter_PART_SRCS})

target_link_libraries(qifimporter
  PUBLIC
    kmm_plugin
  PRIVATE
    KF${QT_MAJOR_VERSION}::Completion
    KF${QT_MAJOR_VERSION}::KIOCore
    KF${QT_MAJOR_VERSION}::KIOWidgets
    Alkimia::alkimia
    kmm_settings
)
