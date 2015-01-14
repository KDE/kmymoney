configure_file(ibanbicdata.desktop.in ibanbicdata.desktop)

set ( PAYEEIDENTIFIER_IBAN_BIC_SCRS
  bicmodel.cpp
  ibanbicdata.cpp
  ibanbic.cpp
)

kde4_add_plugin( payeeidentifier_iban_bic SHARED
  ${PAYEEIDENTIFIER_IBAN_BIC_SCRS}
)

target_link_libraries( payeeidentifier_iban_bic
  kmm_payeeidentifier
  Qt5::Core
  Qt5::Sql
  Qt5::Xml
  KF5::KDELibs4Support
  Alkimia::alkimia
)

install(TARGETS payeeidentifier_iban_bic
        ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/ibanbicdata.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR})

# Storage plugin

configure_file(kmymoney-ibanbic-storageplugin.desktop.in kmymoney-ibanbic-storageplugin.desktop)

set ( IBAN_BIC_STORAGE_PLUGIN_SCRS
  ibanbicstorageplugin.cpp
)

kde4_add_plugin( payeeidentifier_ibanbic_storageplugin
  ${IBAN_BIC_STORAGE_PLUGIN_SCRS}
)

target_link_libraries( payeeidentifier_ibanbic_storageplugin
  Qt5::Core
  Qt5::Sql
  kmm_mymoney
)

install(TARGETS payeeidentifier_ibanbic_storageplugin
  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-ibanbic-storageplugin.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
  )

if( KDE4_BUILD_TESTS )
  add_subdirectory(tests)
endif()

add_subdirectory(widgets)
