add_subdirectory( interfaces )

add_subdirectory( csvimport )

add_subdirectory( csvexport )

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

########### next target ###############

set(kmm_plugin_LIB_SRCS
  importinterface.cpp
  kmymoneyplugin.cpp
  pluginloader.cpp
  statementinterface.cpp viewinterface.cpp)

set(plugins_HEADERS
  importinterface.h kmymoneyplugin.h pluginloader.h
  statementinterface.h viewinterface.h ${CMAKE_CURRENT_BINARY_DIR}/kmm_plugin_export.h)

kde4_add_library(kmm_plugin SHARED ${kmm_plugin_LIB_SRCS})
generate_export_header(kmm_plugin BASE_NAME kmm_plugin)
target_link_libraries(kmm_plugin LINK_PUBLIC ${QT_AND_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} ${LIBALKIMIA_LIBRARIES})

set_target_properties(kmm_plugin PROPERTIES VERSION ${GENERIC_LIB_VERSION}  SOVERSION ${GENERIC_LIB_SOVERSION} )

########### install files ###############

install(TARGETS kmm_plugin
  ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES ${plugins_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney  COMPONENT Devel)

install(FILES kmymoneyplugin.desktop kmymoneyimporterplugin.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR})
