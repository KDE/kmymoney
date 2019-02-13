configure_file(ibanbicdata.desktop.in ibanbicdata.desktop)

set ( PAYEEIDENTIFIER_IBAN_BIC_SCRS
  bicmodel.cpp
  ibanbicdata.cpp
  ibanbic.cpp
)

kde4_add_library( payeeidentifier_iban_bic SHARED
  ${PAYEEIDENTIFIER_IBAN_BIC_SCRS}
)
generate_export_header(payeeidentifier_iban_bic BASE_NAME iban_bic_identifier)

target_link_libraries( payeeidentifier_iban_bic PUBLIC
  kmm_payeeidentifier
  ${QT_QTCORE_LIBRARY}
  ${QT_QTSQL_LIBRARY}
  ${QT_QTXML_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${GMP_LIBRARIES}
)

add_dependencies(payeeidentifier_iban_bic ibanbicdata_de ibanbicdata_ch)

set_target_properties(payeeidentifier_iban_bic PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

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
  ${QT_QTCORE_LIBRARY}
  ${QT_QTSQL_LIBRARY}
  kmm_mymoney
)

install(TARGETS payeeidentifier_ibanbic_storageplugin
  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-ibanbic-storageplugin.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
  )

install( FILES ibanbic.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney/payeeidentifier/ibanandbic COMPONENT Devel)

if( KDE4_BUILD_TESTS )
  add_subdirectory(tests)
endif()

add_subdirectory(widgets)
