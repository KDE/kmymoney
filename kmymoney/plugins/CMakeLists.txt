ADD_SUBDIRECTORY( interfaces )

#ADD_SUBDIRECTORY( csvimport )

#ADD_SUBDIRECTORY( csvexport )

if(LIBOFX_FOUND)
  #ADD_SUBDIRECTORY( ofximport )
endif(LIBOFX_FOUND)

IF (LIBICAL_FOUND)
  #ADD_SUBDIRECTORY( icalendarexport )
ENDIF (LIBICAL_FOUND)

#ADD_SUBDIRECTORY( reconciliationreport )
#ADD_SUBDIRECTORY( printcheck )

IF (KBANKING_FOUND)
  #ADD_SUBDIRECTORY( kbanking )
ENDIF (KBANKING_FOUND)

option(ENABLE_SQLCIPHER "Enable SQLCipher plugin" OFF)
if (ENABLE_SQLCIPHER)
  add_subdirectory(sqlcipher)
endif(ENABLE_SQLCIPHER)

########### next target ###############

SET(kmm_plugin_LIB_SRCS
  importinterface.cpp
  kmymoneyplugin.cpp
  pluginloader.cpp
  statementinterface.cpp viewinterface.cpp)

SET(plugins_HEADERS
  importinterface.h kmymoneyplugin.h pluginloader.h
  statementinterface.h viewinterface.h ${CMAKE_CURRENT_BINARY_DIR}/kmm_plugin_export.h)

add_library(kmm_plugin SHARED ${kmm_plugin_LIB_SRCS})
generate_export_header(kmm_plugin BASE_NAME kmm_plugin)
TARGET_LINK_LIBRARIES(kmm_plugin PUBLIC KF5::KCMUtils KF5::KIOWidgets KF5::XmlGui kmm_mymoney Alkimia::alkimia)

SET_TARGET_PROPERTIES(kmm_plugin PROPERTIES VERSION ${GENERIC_LIB_VERSION}  SOVERSION ${GENERIC_LIB_SOVERSION} )

########### install files ###############

INSTALL(TARGETS kmm_plugin
  ${INSTALL_TARGETS_DEFAULT_ARGS} )

INSTALL(FILES ${plugins_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney  COMPONENT Devel)

INSTALL(FILES kmymoneyplugin.desktop kmymoneyimporterplugin.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR})
