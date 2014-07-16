add_subdirectory( ibanandbic )
add_subdirectory( nationalaccount )

## PayeeIdentifiers

set ( PAYEEIDENTIFIER_HEADER
  payeeidentifier.h
)

set ( PAYEEIDENTIFIER_SCRS
  payeeidentifier.cpp
)

add_library( kmm_payeeidentifier SHARED
  ${PAYEEIDENTIFIER_SCRS}
)

target_link_libraries( kmm_payeeidentifier
  ${QT_QTCORE_LIBRARY}
)

## Plugin loader

set ( PAYEEIDENTIFIER_LOADER_SCRS
  payeeidentifierloader.cpp
)

add_library( kmm_payeeidentifier_loader STATIC
  ${PAYEEIDENTIFIER_LOADER_SCRS}
)

target_link_libraries( kmm_payeeidentifier_loader
  payeeidentifier_iban_bic
  payeeidentifier_iban_bic_widgets
  payeeidentifier_nationalAccount
  ${QT_QTCORE_LIBRARY}
)
