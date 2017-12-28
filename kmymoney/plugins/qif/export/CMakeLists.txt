# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/qifexporter.json.in ${CMAKE_CURRENT_BINARY_DIR}/qifexporter.json @ONLY)

########### next target ###############

set(qifexporter_PART_SRCS
  qifexporter.cpp
  ../config/mymoneyqifprofile.cpp
  mymoneyqifwriter.cpp
  kexportdlg.cpp
  ${CMAKE_SOURCE_DIR}/kmymoney/widgets/kmymoneyaccountcombo.cpp
)

set(qifexporter_PART_UI
  kexportdlgdecl.ui
)

ki18n_wrap_ui(qifexporter_PART_SRCS ${qifexporter_PART_UI})

add_library(qifexporter MODULE ${qifexporter_PART_SRCS})

target_link_libraries(qifexporter
  kmm_plugin
  models #TODO: Get rid of this big dependency
)
                      
########### install files ###############

install(FILES qifexporter.rc
        DESTINATION "${KXMLGUI_INSTALL_DIR}/qifexporter")

install(TARGETS qifexporter
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney/")
