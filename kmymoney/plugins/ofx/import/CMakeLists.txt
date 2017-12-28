# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ofximporter.json.in ${CMAKE_CURRENT_BINARY_DIR}/ofximporter.json @ONLY)

if(${LIBOFX_HAVE_CLIENTUID})
  add_definitions(-DLIBOFX_HAVE_CLIENTUID)
endif(${LIBOFX_HAVE_CLIENTUID})

add_subdirectory( dialogs )

########### next target ###############

set(ofximporter_PART_SRCS
  ofximporter.cpp
  ofxpartner.cpp
)

set(ofximporter_PART_UI
  importoption.ui
)

ki18n_wrap_ui(ofximporter_PART_SRCS ${ofximporter_PART_UI})

add_library(ofximporter MODULE ${ofximporter_PART_SRCS})

target_link_libraries(ofximporter
  ofximporter_dialogs
  kmm_mymoney
  kmm_widgets
  kmm_plugin
  KF5::Wallet
  Qt5::Xml
  Alkimia::alkimia
  ${LIBOFX_LIBRARIES}
)

target_include_directories(ofximporter
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
  ${CMAKE_CURRENT_BINARY_DIR}/dialogs
  ${LIBOFX_INCLUDE_DIR}
)
                      
########### install files ###############

install(FILES ofximporter.rc
        DESTINATION "${KXMLGUI_INSTALL_DIR}/ofximporter")

install(TARGETS ofximporter
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney/")
