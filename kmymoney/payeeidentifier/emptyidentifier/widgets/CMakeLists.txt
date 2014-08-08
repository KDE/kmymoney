set ( EMPTY_WIDGETS_SCRS
  typeselectiondelegate.cpp
)

kde4_add_library( payeeidentifier_empty_widgets STATIC
  ${EMPTY_WIDGETS_SCRS}
)

target_link_libraries( payeeidentifier_empty_widgets
  payeeidentifier_iban_bic
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  kmm_widgets
)

install(TARGETS payeeidentifier_empty_widgets
        ${INSTALL_TARGETS_DEFAULT_ARGS})
