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

add_library(gncimporter MODULE ${gncimporter_PART_SRCS})

target_link_libraries(gncimporter
  kmm_plugin
  Alkimia::alkimia
)

########### install files ###############

install(TARGETS gncimporter
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney/")
