# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gncimport.json.in ${CMAKE_CURRENT_BINARY_DIR}/gncimport.json @ONLY)

########### next target ###############

set(kmm_gncimport_PART_SRCS
  gncimporterplugin.cpp
  kgncimportoptionsdlg.cpp
  kgncpricesourcedlg.cpp
  ../../../widgets/kmymoneymoneyvalidator.cpp
  mymoneygncreader.cpp
)

set(kmm_gncimport_PART_UI
  kgncimportoptionsdlg.ui
  kgncpricesourcedlg.ui
)

ki18n_wrap_ui(kmm_gncimport_PART_SRCS ${kmm_gncimport_PART_UI})

add_library(kmm_gncimport MODULE ${kmm_gncimport_PART_SRCS})

target_link_libraries(kmm_gncimport
  kmm_plugin
  Alkimia::alkimia
)
                      
########### install files ###############

install(FILES kmm_gncimport.rc
        DESTINATION "${KXMLGUI_INSTALL_DIR}/kmm_gncimport")

install(TARGETS kmm_gncimport
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney/")
