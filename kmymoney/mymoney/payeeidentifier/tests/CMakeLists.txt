include(ECMAddTests)

file(GLOB tests_sources "*-test.cpp")
ecm_add_tests(${tests_sources}
  NAME_PREFIX
   payeeidentifier
  LINK_LIBRARIES
    Qt5::Test
    kmm_payeeidentifier
    kmm_mymoney
)
