configure_file(kmymoney-ibanandbic-delegate.desktop.in kmymoney-ibanandbic-delegate.desktop)

set ( IBAN_BIC_WIDGETS_SCRS
  kibanlineedit.cpp
  kbicedit.cpp
  ibanvalidator.cpp
  bicvalidator.cpp
  ibanbicitemdelegate.cpp
  ibanbicitemedit.ui
  ibanbicitemedit.cpp
  pluginfactory.cpp
)

kde4_add_ui_files(
  IBAN_BIC_WIDGETS_SCRS
  ibanbicitemedit.ui
)

kde4_add_plugin( payeeidentifier_iban_bic_widgets
  ${IBAN_BIC_WIDGETS_SCRS}
)

target_link_libraries( payeeidentifier_iban_bic_widgets
  payeeidentifier_iban_bic
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  kmm_widgets
)

install(TARGETS payeeidentifier_iban_bic_widgets
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-ibanandbic-delegate.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})