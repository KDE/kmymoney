configure_file(ibanbicdata.desktop.in ibanbicdata.desktop)

set ( PAYEEIDENTIFIER_IBAN_BIC_SCRS
  bicmodel.cpp
  ibanbicdata.cpp
  ibanbic.cpp
)

kde4_add_plugin( payeeidentifier_iban_bic SHARED
  ${PAYEEIDENTIFIER_IBAN_BIC_SCRS}
)

target_link_libraries( payeeidentifier_iban_bic
  kmm_payeeidentifier
  ${QT_QTCORE_LIBRARY}
  ${QT_QTSQL_LIBRARY}
  ${QT_QTXML_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${GMP_LIBRARIES}
)

install(TARGETS payeeidentifier_iban_bic
        ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ibanbicdata.desktop
        DESTINATION ${SERVICETYPES_INSTALL_DIR})

if( KDE4_BUILD_TESTS )
  add_subdirectory(tests)
endif()

add_subdirectory(widgets)