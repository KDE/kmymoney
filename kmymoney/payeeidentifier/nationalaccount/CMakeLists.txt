
set (nationalAccountId_SCRS
  nationalaccount.cpp
)

add_library( payeeidentifier_nationalAccount SHARED
  ${nationalAccountId_SCRS}
)
generate_export_header(payeeidentifier_nationalAccount BASE_NAME nationalaccount_identifier)

target_link_libraries( payeeidentifier_nationalAccount
  PUBLIC
    kmm_payeeidentifier
    Qt5::Core
    Qt5::Xml
    Qt5::Sql
)

set_target_properties(payeeidentifier_nationalAccount PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmymoney-nationalaccountnumberplugin.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-nationalaccountnumberplugin.desktop)
kcoreaddons_desktop_to_json(payeeidentifier_nationalAccount ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-nationalaccountnumberplugin.desktop)

install(TARGETS payeeidentifier_nationalAccount
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-nationalaccountnumberplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

add_subdirectory( ui )
