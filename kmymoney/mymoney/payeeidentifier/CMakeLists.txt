set ( PAYEEIDENTIFIER_HEADER
  payeeidentifier.h
  payeeidentifiertyped.h
  payeeidentifierdata.h
)

set ( PAYEEIDENTIFIER_SCRS
  payeeidentifier.cpp
)

add_library( kmm_payeeidentifier SHARED
  ${PAYEEIDENTIFIER_SCRS}
)

generate_export_header( kmm_payeeidentifier )

target_link_libraries( kmm_payeeidentifier
  Qt5::Sql
  Qt5::Core
  Qt5::Xml
)

target_include_directories( kmm_payeeidentifier PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
)

install( TARGETS kmm_payeeidentifier
  ${INSTALL_TARGETS_DEFAULT_ARGS})

## Plugin loader

set ( PAYEEIDENTIFIER_LOADER_SCRS
  payeeidentifierloader.cpp
)

add_library( kmm_payeeidentifier_loader STATIC
  ${PAYEEIDENTIFIER_LOADER_SCRS}
)

target_link_libraries( kmm_payeeidentifier_loader
  kmm_payeeidentifier
  payeeidentifier_iban_bic
  payeeidentifier_nationalAccount
  payeeidentifier_unavailable
  KF5::I18n
  Qt5::Widgets
  Qt5::Core
)

add_subdirectory(tests)
