# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ofximporter.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/ofximporter.json @ONLY)

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

qt5_add_resources(ofximporter_PART_SRCS ofximporter.qrc)

kcoreaddons_add_plugin(ofximporter
                       SOURCES ${ofximporter_PART_SRCS}
                       INSTALL_NAMESPACE "kmymoney")

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(ofximporter
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

target_link_libraries(ofximporter
  ofximporter_dialogs
  kmm_mymoney
  kmm_widgets
  kmm_plugin
  KF5::Wallet
  Qt5::Xml
  Alkimia::alkimia
  PkgConfig::LIBOFX
)

target_include_directories(ofximporter
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
  ${CMAKE_CURRENT_BINARY_DIR}/dialogs
  ${LIBOFX_INCLUDEDIR}
)
