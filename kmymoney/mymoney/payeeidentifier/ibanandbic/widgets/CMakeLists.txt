set ( IBAN_BIC_WIDGETS_SCRS
  kbicedit.cpp
)

kde4_add_library( payeeidentifier_iban_bic_widgets SHARED
  ${IBAN_BIC_WIDGETS_SCRS}
)

target_link_libraries( payeeidentifier_iban_bic_widgets
  payeeidentifier_iban_bic
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
)

install(TARGETS payeeidentifier_iban_bic_widgets
        ${INSTALL_TARGETS_DEFAULT_ARGS})
