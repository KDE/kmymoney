# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ofximport.json.in ${CMAKE_CURRENT_BINARY_DIR}/ofximport.json @ONLY)

if(${LIBOFX_HAVE_CLIENTUID})
  add_definitions(-DLIBOFX_HAVE_CLIENTUID)
endif(${LIBOFX_HAVE_CLIENTUID})

add_subdirectory( dialogs )

########### next target ###############

set(kmm_ofximport_PART_SRCS
  ofximporterplugin.cpp
  ofxpartner.cpp
)

set(kmm_ofximport_PART_UI
  importoption.ui
)

ki18n_wrap_ui(kmm_ofximport_PART_SRCS ${kmm_ofximport_PART_UI})

add_library(kmm_ofximport MODULE ${kmm_ofximport_PART_SRCS})

target_link_libraries(kmm_ofximport
  kmm_ofximport_dialogs
  kmm_mymoney
  kmm_widgets
  kmm_plugin
  KF5::Wallet
  Qt5::Xml
  Alkimia::alkimia
  ${LIBOFX_LIBRARIES}
)

target_include_directories(kmm_ofximport
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
  ${CMAKE_CURRENT_BINARY_DIR}/dialogs
  ${LIBOFX_INCLUDE_DIR}
)
                      
########### install files ###############

install(FILES kmm_ofximport.rc
        DESTINATION "${KXMLGUI_INSTALL_DIR}/kmm_ofximport")

install(TARGETS kmm_ofximport
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney/")
