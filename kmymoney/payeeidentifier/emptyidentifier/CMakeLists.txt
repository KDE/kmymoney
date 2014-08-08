set ( EMPTYIDENTIFIER_SCRS
  empty.cpp
)

kde4_add_plugin( payeeidentifier_empty STATIC
  ${EMPTYIDENTIFIER_SCRS}
)

target_link_libraries( payeeidentifier_empty
  kmm_payeeidentifier
  ${QT_QTCORE_LIBRARY}
  ${QT_QTSQL_LIBRARY}
  ${QT_QTXML_LIBRARY}
  ${KDE4_KDECORE_LIBS}
)

install(TARGETS payeeidentifier_empty
        ${INSTALL_TARGETS_DEFAULT_ARGS}
)

add_subdirectory(widgets)
