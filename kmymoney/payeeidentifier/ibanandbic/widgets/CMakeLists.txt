## Shared library for widgets and modulue for delegate
# Note: all "real" code is in ..._widgets, ..._delegate is only used to load the shared library

set ( IBAN_BIC_WIDGETS_SCRS
  kibanlineedit.cpp
  kbicedit.cpp
  ibanvalidator.cpp
  bicvalidator.cpp
  ibanbicitemdelegate.cpp
  ibanbicitemedit.cpp
)

set( IBAN_BIC_WIDGETS_HEADERS
  kibanlineedit.h
  kbicedit.h
  ibanvalidator.h
  bicvalidator.h
  ibanbicitemdelegate.h
  ${CMAKE_CURRENT_BINARY_DIR}/payeeidentifier_iban_bic_widgets_export.h
)

ki18n_wrap_ui(IBAN_BIC_WIDGETS_SCRS
  ibanbicitemedit.ui
)

add_library( payeeidentifier_iban_bic_widgets SHARED
  ${IBAN_BIC_WIDGETS_SCRS}
)

set_target_properties(payeeidentifier_iban_bic_widgets
  PROPERTIES VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(payeeidentifier_iban_bic_widgets
  PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries( payeeidentifier_iban_bic_widgets PUBLIC
  payeeidentifier_iban_bic
  Qt5::Core
  kmm_mymoney
  kmm_widgets
)

generate_export_header( payeeidentifier_iban_bic_widgets )

install(TARGETS payeeidentifier_iban_bic_widgets
        ${INSTALL_TARGETS_DEFAULT_ARGS}
)

## Delegate plugin
set (IBAN_BIC_UI_SCRS
  pluginfactory.cpp
)

add_library( payeeidentifier_iban_bic_delegates MODULE
  ${IBAN_BIC_UI_SCRS}
)

target_link_libraries( payeeidentifier_iban_bic_delegates
  payeeidentifier_iban_bic
  payeeidentifier_iban_bic_widgets
  kmm_widgets
  Qt5::Core
)

configure_file(kmymoney-ibanbic-delegate.desktop.in kmymoney-ibanbic-delegate.desktop)
kcoreaddons_desktop_to_json(payeeidentifier_iban_bic_widgets "${CMAKE_CURRENT_BINARY_DIR}/kmymoney-ibanbic-delegate.desktop")

install(TARGETS payeeidentifier_iban_bic_delegates
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney"
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-ibanbic-delegate.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
)
