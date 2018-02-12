set( _HEADERS kmymoneyutils.h kmymoneyglobalsettings.h )

########### settings code (kmm_settings) STATIC ###############
kde4_add_kcfg_files( kmm_settings_SRCS kmymoneysettings.kcfgc )
kde4_add_library(kmm_settings SHARED ${kmm_settings_SRCS})
generate_export_header(kmm_settings)
target_link_libraries(kmm_settings
  PRIVATE
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
)
install(TARGETS kmm_settings ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES kmymoney.kcfg DESTINATION ${KCFG_INSTALL_DIR})
