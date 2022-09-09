set (menus_SOURCES
  menuenums.cpp
)

add_library(kmm_menus ${menus_SOURCES})
generate_export_header(kmm_menus)
target_link_libraries(kmm_menus PRIVATE Qt::Widgets Qt::Core)

set_target_properties(kmm_menus PROPERTIES
  VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
)

install(TARGETS kmm_menus
  ${INSTALL_TARGETS_DEFAULT_ARGS} )
