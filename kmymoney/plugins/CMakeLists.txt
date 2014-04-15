add_subdirectory( onlinetasks )

add_subdirectory( ibanbicdata )

ADD_SUBDIRECTORY( interfaces )

ADD_SUBDIRECTORY( csvimport )

ADD_SUBDIRECTORY( csvexport )

if(LIBOFX_FOUND)
  ADD_SUBDIRECTORY( ofximport )
endif(LIBOFX_FOUND)

IF (LIBICAL_FOUND)
  ADD_SUBDIRECTORY( icalendarexport )
ENDIF (LIBICAL_FOUND)

ADD_SUBDIRECTORY( reconciliationreport )
ADD_SUBDIRECTORY( printcheck )

IF (KBANKING_FOUND)
  ADD_SUBDIRECTORY( kbanking )
ENDIF (KBANKING_FOUND)

########### next target ###############

SET(kmm_plugin_LIB_SRCS
  importinterface.cpp 
  kmymoneyplugin.cpp 
  pluginloader.cpp
  statementinterface.cpp
  viewinterface.cpp
  onlinepluginextended.cpp
)

SET(plugins_HEADERS
  importinterface.h kmymoneyplugin.h pluginloader.h
  statementinterface.h viewinterface.h kmm_plugin_export.h onlinepluginextended.h)

KDE4_ADD_LIBRARY(kmm_plugin SHARED ${kmm_plugin_LIB_SRCS} ${plugins_HEADERS})
TARGET_LINK_LIBRARIES(kmm_plugin
    ${QT_AND_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KUTILS_LIBS}
    ${GMP_LIBRARIES}
    ${LIBALKIMIA_LIBRARY}
    kmymoney_common
)

SET_TARGET_PROPERTIES(kmm_plugin PROPERTIES VERSION ${GENERIC_LIB_VERSION}  SOVERSION ${GENERIC_LIB_SOVERSION} )

########### install files ###############

INSTALL(TARGETS kmm_plugin 
  ${INSTALL_TARGETS_DEFAULT_ARGS} )

INSTALL(FILES ${plugins_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney  COMPONENT Devel)

INSTALL(FILES kmymoneyplugin.desktop kmymoneyimporterplugin.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR})
