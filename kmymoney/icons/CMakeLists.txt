include(ECMInstallIcons)

file(GLOB_RECURSE KMYMONEY_HICOLOR_ICONS "hicolor/*")
ecm_install_icons(
  ICONS ${KMYMONEY_HICOLOR_ICONS}
  DESTINATION ${ICON_INSTALL_DIR} # or stay to ${DATA_INSTALL_DIR}/kmymoney/icons ?
  THEME "hicolor"
)

file(GLOB_RECURSE KMYMONEY_OXYGEN_ICONS "oxygen/*")
ecm_install_icons(
  ICONS ${KMYMONEY_OXYGEN_ICONS}
  DESTINATION ${ICON_INSTALL_DIR}
  THEME "oxygen"
)

file(GLOB_RECURSE KMYMONEY_ICONS "kmymoney/*.png")
ecm_install_icons(
  ICONS ${KMYMONEY_ICONS}
  DESTINATION ${ICON_INSTALL_DIR}
)
