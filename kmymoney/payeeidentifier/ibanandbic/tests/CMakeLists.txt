
set( payeeidentifierIbanBicTest_SCRS
   internationalaccountidentifiertest.cpp
)

kde4_add_unit_test( payeeidentifier_iban_bic-test
  TESTNAME kmymoney-payeeidentifier_iban_bic
  ${payeeidentifierIbanBicTest_SCRS}
)

target_link_libraries( payeeidentifier_iban_bic-test
  ${QT_QTTEST_LIBRARY}
  kmm_payeeidentifier_iban_bic
)
