ADD_SUBDIRECTORY( interfaces ) 

if(USE_OFX_DIRECTCONNECT)
	ADD_SUBDIRECTORY( ofximport )
endif(USE_OFX_DIRECTCONNECT)


########### next target ###############

SET(kmm_plugin_LIB_SRCS
  importinterface.cpp 
  kmymoneyplugin.cpp 
  pluginloader.cpp
  statementinterface.cpp viewinterface.cpp)

SET(plugins_HEADERS
  importinterface.h kmymoneyplugin.h pluginloader.h
  statementinterface.h viewinterface.h)


KDE4_ADD_LIBRARY(kmm_plugin SHARED ${kmm_plugin_LIB_SRCS})
TARGET_LINK_LIBRARIES(kmm_plugin ${QT_AND_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS})

SET_TARGET_PROPERTIES(kmm_plugin PROPERTIES VERSION ${GENERIC_LIB_VERSION}  SOVERSION ${GENERIC_LIB_SOVERSION} )

########### install files ###############

INSTALL(TARGETS kmm_plugin 
	${INSTALL_TARGETS_DEFAULT_ARGS} )

INSTALL(FILES ${plugins_HEADERS}
	DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney  COMPONENT Devel)

INSTALL(FILES kmymoneyplugin.desktop kmymoneyimporterplugin.desktop
  DESTINATION share/servicetypes)
