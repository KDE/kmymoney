add_subdirectory( onlinetasks )
add_subdirectory( interfaces )
add_subdirectory( csv )
add_subdirectory( qif )
add_subdirectory( gnc )
add_subdirectory( ofx )
add_subdirectory( icalendar )
add_subdirectory( reconciliationreport )
add_subdirectory( checkprinting )
add_subdirectory( views )
add_subdirectory( xml )
add_subdirectory( xmlhelper )

if(ENABLE_SQLSTORAGE)
  add_subdirectory(sql)
endif()

if(ENABLE_IBANBICDATA)
  add_subdirectory(ibanbicdata)
endif()

if (ENABLE_KBANKING)
  add_subdirectory( kbanking )
endif (ENABLE_KBANKING)

if (ENABLE_SQLCIPHER)
  add_subdirectory(sqlcipher)
endif()

option(ENABLE_ONLINEJOBPLUGINMOCKUP "Enable onlineJob-plugin mockup (only for developers)" OFF)
if (ENABLE_ONLINEJOBPLUGINMOCKUP)
  add_subdirectory(onlinejobpluginmockup)
endif()

if (ENABLE_WEBOOB)
  add_subdirectory( weboob )
endif ()

########### next target ###############

set(kmm_plugin_LIB_SRCS
  appinterface.cpp
  importinterface.cpp
  kmymoneyplugin.cpp
  statementinterface.cpp
  viewinterface.cpp
  onlinepluginextended.cpp
  interfaceloader.cpp
)

set(plugin_HEADERS
  appinterface.h
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
    Qt5::Core
    KF5::XmlGui
  PRIVATE
    KF5::KCMUtils
    KF5::KIOWidgets
    Qt5::Gui
    Qt5::Widgets
  INTERFACE
    kmm_mymoney
)

set_target_properties(kmm_plugin PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

########### install files ###############

install(TARGETS kmm_plugin
  ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES ${plugin_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney  COMPONENT Devel)

