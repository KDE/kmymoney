add_subdirectory( interfaces )

ADD_SUBDIRECTORY( csvimport )

ADD_SUBDIRECTORY( csvexport )

if(LIBOFX_FOUND)
  #ADD_SUBDIRECTORY( ofximport )
endif(LIBOFX_FOUND)

if (LIBICAL_FOUND)
  ADD_SUBDIRECTORY( icalendarexport )
endif (LIBICAL_FOUND)

ADD_SUBDIRECTORY( reconciliationreport )
ADD_SUBDIRECTORY( printcheck )

if (KBANKING_FOUND)
  ADD_SUBDIRECTORY( kbanking )
endif (KBANKING_FOUND)

option(ENABLE_SQLCIPHER "Enable SQLCipher plugin" OFF)
if (ENABLE_SQLCIPHER)
  add_subdirectory(sqlcipher)
endif(ENABLE_SQLCIPHER)

########### next target ###############

set(kmm_plugin_LIB_SRCS
  importinterface.cpp
  kmymoneyplugin.cpp
  pluginloader.cpp
  statementinterface.cpp viewinterface.cpp)

set(plugins_HEADERS
  importinterface.h kmymoneyplugin.h pluginloader.h
  statementinterface.h viewinterface.h ${CMAKE_CURRENT_BINARY_DIR}/kmm_plugin_export.h)

add_library(kmm_plugin SHARED ${kmm_plugin_LIB_SRCS})
generate_export_header(kmm_plugin BASE_NAME kmm_plugin)
target_link_libraries(kmm_plugin PUBLIC KF5::KCMUtils KF5::KIOWidgets KF5::XmlGui KF5::KDELibs4Support Qt5::Gui kmm_mymoney Alkimia::alkimia)

set_target_properties(kmm_plugin PROPERTIES VERSION ${GENERIC_LIB_VERSION}  SOVERSION ${GENERIC_LIB_SOVERSION} )

########### install files ###############

install(TARGETS kmm_plugin
  ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES ${plugins_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney  COMPONENT Devel)

install(FILES kmymoneyplugin.desktop kmymoneyimporterplugin.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR})
