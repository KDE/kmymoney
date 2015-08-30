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
  ${QT_QTCORE_LIBRARY}
  ${QT_QTXML_LIBRARY}
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
  kmm_payeeidentifier_iban_bic
  payeeidentifier_nationalAccount
  payeeidentifier_unavailable
  ${QT_QTCORE_LIBRARY}
)
