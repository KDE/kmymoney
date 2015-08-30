## Shared library for widgets

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

kde4_add_ui_files(IBAN_BIC_WIDGETS_SCRS
  ibanbicitemedit.ui
)

kde4_add_library( payeeidentifier_iban_bic_widgets SHARED
  ${IBAN_BIC_WIDGETS_SCRS}
)

target_include_directories(payeeidentifier_iban_bic_widgets
  PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries( payeeidentifier_iban_bic_widgets LINK_PUBLIC
  kmm_payeeidentifier_iban_bic
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  kmm_mymoney
  kmm_widgets
)

generate_export_header( payeeidentifier_iban_bic_widgets )

install(TARGETS payeeidentifier_iban_bic_widgets
        ${INSTALL_TARGETS_DEFAULT_ARGS})

## Delegate plugin
set (IBAN_BIC_UI_SCRS
  pluginfactory.cpp
)

kde4_add_plugin( payeeidentifier_iban_bic_delegates
  ${IBAN_BIC_UI_SCRS}
)

target_link_libraries( payeeidentifier_iban_bic_delegates
  kmm_payeeidentifier_iban_bic
  payeeidentifier_iban_bic_widgets
  kmm_widgets
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDEUI_LIBS}
)

install(TARGETS payeeidentifier_iban_bic_delegates
        DESTINATION ${PLUGIN_INSTALL_DIR})

configure_file(kmymoney-ibanbic-delegate.desktop.in kmymoney-ibanbic-delegate.desktop)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-ibanbic-delegate.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
