# Storage plugin

set ( sepa_STORAGE_PLUGIN_SCRS
  sepastorageplugin.cpp
)

add_library( credittransfer_sepa_storageplugin MODULE
  ${sepa_STORAGE_PLUGIN_SCRS}
)

target_link_libraries( credittransfer_sepa_storageplugin
  Qt5::Core
  Qt5::Sql
  kmm_mymoney
)

configure_file(kmymoney-sepa-storageplugin.desktop.in kmymoney-sepa-storageplugin.desktop)
kcoreaddons_desktop_to_json(credittransfer_sepa_storageplugin
  "${CMAKE_CURRENT_BINARY_DIR}/kmymoney-sepa-storageplugin.desktop"
  SERVICE_TYPES "${KMyMoney_SOURCE_DIR}/kmymoney/plugins/sql/kmymoney-sqlstorageplugin.desktop"
)

install(TARGETS credittransfer_sepa_storageplugin
  DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney")

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-sepa-storageplugin.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
)
