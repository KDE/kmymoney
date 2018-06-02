# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gncimporter.json.in ${CMAKE_CURRENT_BINARY_DIR}/gncimporter.json @ONLY)

########### next target ###############

set(gncimporter_PART_SRCS
  gncimporter.cpp
  kgncimportoptionsdlg.cpp
  kgncpricesourcedlg.cpp
  ../../../widgets/kmymoneymoneyvalidator.cpp
  mymoneygncreader.cpp
)

set(gncimporter_PART_UI
  kgncimportoptionsdlg.ui
  kgncpricesourcedlg.ui
)

ki18n_wrap_ui(gncimporter_PART_SRCS ${gncimporter_PART_UI})

kcoreaddons_add_plugin(gncimporter
                       SOURCES ${gncimporter_PART_SRCS}
                       JSON "${CMAKE_CURRENT_BINARY_DIR}/gncimporter.json"
                       INSTALL_NAMESPACE "kmymoney")

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(gncimporter
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

target_link_libraries(gncimporter
  PUBLIC
    kmm_plugin
  PRIVATE
    KF5::Completion
    KF5::Archive
    Alkimia::alkimia
)
