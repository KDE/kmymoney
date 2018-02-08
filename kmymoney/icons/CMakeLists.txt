include(ECMInstallIcons)

file(GLOB_RECURSE KMYMONEY_HICOLOR_ICONS "hicolor/*")
ecm_install_icons(
  ICONS ${KMYMONEY_HICOLOR_ICONS}
  DESTINATION ${DATA_INSTALL_DIR}/kmymoney/icons
  THEME "hicolor"
)

file(GLOB_RECURSE KMYMONEY_OXYGEN_ICONS "oxygen/*")
ecm_install_icons(
  ICONS ${KMYMONEY_OXYGEN_ICONS}
  DESTINATION ${DATA_INSTALL_DIR}/kmymoney/icons
  THEME "oxygen"
)

file(GLOB_RECURSE KMYMONEY_TANGO_ICONS "Tango/*")
ecm_install_icons(
  ICONS ${KMYMONEY_TANGO_ICONS}
  DESTINATION ${DATA_INSTALL_DIR}/kmymoney/icons
  THEME "Tango"
)

file(GLOB_RECURSE KMYMONEY_ICONS "kmymoney/*.png")
ecm_install_icons(
  ICONS ${KMYMONEY_ICONS}
  DESTINATION ${ICON_INSTALL_DIR}
)

set (icons_SOURCES
  icons.cpp
)

add_library(kmm_icons SHARED ${icons_SOURCES})
generate_export_header(kmm_icons)
target_link_libraries(kmm_icons PRIVATE Qt5::Core Qt5::Gui)

set_target_properties(kmm_icons PROPERTIES
  VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
)

install(TARGETS kmm_icons
  ${INSTALL_TARGETS_DEFAULT_ARGS} )
