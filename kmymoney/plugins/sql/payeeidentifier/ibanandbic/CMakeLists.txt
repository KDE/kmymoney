#Storage plugin

configure_file(kmymoney-ibanbic-storageplugin.desktop.in kmymoney-ibanbic-storageplugin.desktop)

set ( IBAN_BIC_STORAGE_PLUGIN_SCRS
  ibanbicstorageplugin.cpp
  ../../kmymoneystorageplugin.cpp
)

add_library( payeeidentifier_ibanbic_storageplugin MODULE
  ${IBAN_BIC_STORAGE_PLUGIN_SCRS}
)

target_link_libraries( payeeidentifier_ibanbic_storageplugin
  Qt5::Core
  Qt5::Sql
  kmm_mymoney
)

kcoreaddons_desktop_to_json(payeeidentifier_ibanbic_storageplugin
  "${CMAKE_CURRENT_BINARY_DIR}/kmymoney-ibanbic-storageplugin.desktop"
  SERVICE_TYPES "${KMyMoney_SOURCE_DIR}/kmymoney/plugins/sql/kmymoney-sqlstorageplugin.desktop"
  )

install(TARGETS payeeidentifier_ibanbic_storageplugin
  DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney")

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/kmymoney-ibanbic-storageplugin.desktop"
  DESTINATION ${SERVICES_INSTALL_DIR}
  )
