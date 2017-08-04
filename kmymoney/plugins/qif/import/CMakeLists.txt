# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/qifimport.json.in ${CMAKE_CURRENT_BINARY_DIR}/qifimport.json @ONLY)

########### next target ###############

set(kmm_qifimport_PART_SRCS
  qifimporterplugin.cpp
  ../config/mymoneyqifprofile.cpp
  mymoneyqifreader.cpp
  kimportdlg.cpp
)

set(kmm_qifimport_PART_UI
  kimportdlgdecl.ui
)

ki18n_wrap_ui(kmm_qifimport_PART_SRCS ${kmm_qifimport_PART_UI})

add_library(kmm_qifimport MODULE ${kmm_qifimport_PART_SRCS})

target_link_libraries(kmm_qifimport
  kmm_plugin
  Alkimia::alkimia
)
                      
########### install files ###############

install(FILES kmm_qifimport.rc
        DESTINATION "${KXMLGUI_INSTALL_DIR}/kmm_qifimport")

install(TARGETS kmm_qifimport
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney/")
