
kconfig_add_kcfg_files( kmm_settings_SRCS kmymoneysettings.kcfgc )

add_library(kmm_settings ${kmm_settings_SRCS})

set_target_properties(kmm_settings PROPERTIES
      VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR} )

generate_export_header(kmm_settings BASE_NAME kmm_settings)

target_link_libraries(kmm_settings
                      PRIVATE
                      KF5::ConfigWidgets)

set(kmm_settings_HEADERS
    ${CMAKE_CURRENT_BINARY_DIR}/kmm_settings_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/kmymoneysettings.h
    kmymoneysettings_addons.h)


install(TARGETS kmm_settings
  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES kmymoney.kcfg
  DESTINATION ${KDE_INSTALL_KCFGDIR}
  )
install(FILES kmymoney.upd
  DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR}
  )

install(FILES ${kmm_settings_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kmymoney COMPONENT Devel)

