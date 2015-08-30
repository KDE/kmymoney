set( germanonlinetransfertest_SRCS
  germanonlinetransfertest.cpp
  )

kde4_add_unit_test( germanonlinetransfertest
  TESTNAME kmymoney-germanOnlineTransferTest
  ${germanonlinetransfertest_SRCS}
  $<TARGET_OBJECTS:konlinetasks_national_OBJECTS>
  $<TARGET_OBJECTS:konlinetasks_sepa_OBJECTS>
)

add_dependencies( germanonlinetransfertest
  konlinetasks_national
  konlinetasks_sepa
)

target_link_libraries( germanonlinetransfertest
  kmm_mymoney
  kmm_widgets
  kmymoney_base
  kmm_config
  kmm_plugin
  kmm_utils_validators
  onlinetask_interfaces
  payeeidentifier_nationalAccount
  kmm_payeeidentifier_iban_bic
  payeeidentifier_iban_bic_widgets
  payeeidentifier_iban_bic_delegates
  ${QT_QTCORE_LIBRARIES}
  ${QT_QTGUI_LIBRARIES}
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  ${QT_QTXML_LIBRARY}
  ${LIBALKIMIA_LIBRARY}
  ${GMP_LIBRARIES} # TODO: remove when alkimia cmake config file got fixed
#  ${KTOBLZCHECK_LIBRARY}
)
