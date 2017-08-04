# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/qifexport.json.in ${CMAKE_CURRENT_BINARY_DIR}/qifexport.json @ONLY)

########### next target ###############

set(kmm_qifexport_PART_SRCS
  qifexporterplugin.cpp
  ../config/mymoneyqifprofile.cpp
  mymoneyqifwriter.cpp
  kexportdlg.cpp
  ${CMAKE_SOURCE_DIR}/kmymoney/widgets/kmymoneyaccountcombo.cpp
)

set(kmm_qifexport_PART_UI
  kexportdlgdecl.ui
)

ki18n_wrap_ui(kmm_qifexport_PART_SRCS ${kmm_qifexport_PART_UI})

add_library(kmm_qifexport MODULE ${kmm_qifexport_PART_SRCS})

target_link_libraries(kmm_qifexport
  kmm_plugin
  models #TODO: Get rid of this big dependency
)
                      
########### install files ###############

install(FILES kmm_qifexport.rc
        DESTINATION "${KXMLGUI_INSTALL_DIR}/kmm_qifexport")

install(TARGETS kmm_qifexport
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney/")
