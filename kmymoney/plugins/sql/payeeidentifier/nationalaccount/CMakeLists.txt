# Storage plugin

set ( nationalAccount_STORAGE_PLUGIN_SCRS
  nationalaccountstorageplugin.cpp
)

add_library( payeeidentifier_nationalaccount_storageplugin MODULE
  ${nationalAccount_STORAGE_PLUGIN_SCRS}
)

target_link_libraries( payeeidentifier_nationalaccount_storageplugin
  Qt5::Core
  Qt5::Sql
  kmm_mymoney
)

configure_file(kmymoney-nationalaccount-storageplugin.desktop.in kmymoney-nationalaccount-storageplugin.desktop)
kcoreaddons_desktop_to_json(payeeidentifier_nationalaccount_storageplugin
  "${CMAKE_CURRENT_BINARY_DIR}/kmymoney-nationalaccount-storageplugin.desktop"
  SERVICE_TYPES "${KMyMoney_SOURCE_DIR}/kmymoney/plugins/sql/kmymoney-sqlstorageplugin.desktop"
)

install(TARGETS payeeidentifier_nationalaccount_storageplugin
  DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney")

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-nationalaccount-storageplugin.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
)
