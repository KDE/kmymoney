# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/qifimporter.json.in ${CMAKE_CURRENT_BINARY_DIR}/qifimporter.json @ONLY)

########### next target ###############

set(qifimporter_PART_SRCS
  qifimporter.cpp
  ../config/mymoneyqifprofile.cpp
  mymoneyqifreader.cpp
  kimportdlg.cpp
)

set(qifimporter_PART_UI
  kimportdlgdecl.ui
)

ki18n_wrap_ui(qifimporter_PART_SRCS ${qifimporter_PART_UI})

kcoreaddons_add_plugin(qifimporter
                       SOURCES ${qifimporter_PART_SRCS}
                       JSON "${CMAKE_CURRENT_BINARY_DIR}/qifimporter.json"
                       INSTALL_NAMESPACE "kmymoney")

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(qifimporter
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

target_link_libraries(qifimporter
  PUBLIC
    kmm_plugin
  PRIVATE
    KF5::Completion
    KF5::KIOWidgets
    Alkimia::alkimia
    kmm_settings
)

########### install files ###############

install(FILES qifimporter.rc
        DESTINATION "${KXMLGUI_INSTALL_DIR}/qifimporter")
