set ( PAYEEIDENTIFIER_IBAN_BIC_SCRS
  bicmodel.cpp
  ibanbicdata.cpp
  ibanbic.cpp
)

add_library( payeeidentifier_iban_bic SHARED
  ${PAYEEIDENTIFIER_IBAN_BIC_SCRS}
)
generate_export_header(payeeidentifier_iban_bic BASE_NAME iban_bic_identifier)

configure_file(ibanbicdata.desktop.in ibanbicdata.desktop)
kcoreaddons_desktop_to_json(payeeidentifier_iban_bic "${CMAKE_CURRENT_BINARY_DIR}/ibanbicdata.desktop")

target_link_libraries( payeeidentifier_iban_bic PUBLIC
  kmm_payeeidentifier
  KF5::Service
  Qt5::Core
  Qt5::Sql
  Qt5::Xml
  Alkimia::alkimia
)

set_target_properties(payeeidentifier_iban_bic PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

install(TARGETS payeeidentifier_iban_bic
        ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/ibanbicdata.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR})

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
add_subdirectory(widgets)
