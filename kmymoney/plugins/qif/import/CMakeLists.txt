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

add_library(qifimporter MODULE ${qifimporter_PART_SRCS})

target_link_libraries(qifimporter
  kmm_plugin
  Alkimia::alkimia
)
                      
########### install files ###############

install(FILES qifimporter.rc
        DESTINATION "${KXMLGUI_INSTALL_DIR}/qifimporter")

install(TARGETS qifimporter
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney/")
