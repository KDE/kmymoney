add_subdirectory( onlinetasks )
add_subdirectory( ibanbicdata )
add_subdirectory( interfaces )
add_subdirectory( csvimport )
add_subdirectory( csvexport )
add_subdirectory( qif )

if(LIBOFX_FOUND)
  add_subdirectory( ofximport )
endif(LIBOFX_FOUND)

if (LIBICAL_FOUND)
  add_subdirectory( icalendarexport )
endif (LIBICAL_FOUND)

add_subdirectory( reconciliationreport )
add_subdirectory( printcheck )

if (KBANKING_FOUND)
  add_subdirectory( kbanking )
endif (KBANKING_FOUND)

option(ENABLE_SQLCIPHER "Enable SQLCipher plugin" OFF)
if (ENABLE_SQLCIPHER)
  add_subdirectory(sqlcipher)
endif(ENABLE_SQLCIPHER)

option(ENABLE_ONLINEJOBPLUGINMOCKUP "Enable onlineJob-plugin mockup (only for developers)" OFF)
if (ENABLE_ONLINEJOBPLUGINMOCKUP)
  add_subdirectory(onlinejobpluginmockup)
endif()

if (WEBOOB_FOUND)
  add_subdirectory( weboob )
endif (WEBOOB_FOUND)

########### next target ###############

set(kmm_plugin_LIB_SRCS
  importinterface.cpp
  kmymoneyplugin.cpp
  statementinterface.cpp
  viewinterface.cpp
  onlinepluginextended.cpp
  interfaceloader.cpp
)

set(plugins_HEADERS
  importinterface.h
  kmymoneyplugin.h
  statementinterface.h
  viewinterface.h
  ${CMAKE_CURRENT_BINARY_DIR}/kmm_plugin_export.h
  onlinepluginextended.h
)

add_library(kmm_plugin SHARED ${kmm_plugin_LIB_SRCS})
generate_export_header(kmm_plugin BASE_NAME kmm_plugin)
target_link_libraries(kmm_plugin
  PUBLIC
  KF5::XmlGui
  KF5::KCMUtils
  KF5::KIOWidgets
  Qt5::Gui
  Qt5::Widgets
  kmm_mymoney
)

set_target_properties(kmm_plugin PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

########### install files ###############

install(TARGETS kmm_plugin
  ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES ${plugins_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney  COMPONENT Devel)

install(FILES
  kmymoney-payeeidentifierdelegate.desktop
  kmymoney-importerplugin.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR}
)
