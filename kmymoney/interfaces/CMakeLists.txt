
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

add_library(kmm_plugin ${kmm_plugin_LIB_SRCS})
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

