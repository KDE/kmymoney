# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gncimporter.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/gncimporter.json @ONLY)

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

kmymoney_add_plugin(gncimporter SOURCES ${gncimporter_PART_SRCS})

target_link_libraries(gncimporter
  PUBLIC
    kmm_plugin
  PRIVATE
    kmm_models
    KF5::Completion
    KF5::Archive
    Alkimia::alkimia
    kmm_yesno
)
