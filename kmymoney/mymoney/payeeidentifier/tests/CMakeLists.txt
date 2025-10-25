file(GLOB tests_sources "test-*.cpp")
kmm_add_tests(${tests_sources}
  NAME_PREFIX
   payeeidentifier
  LINK_LIBRARIES
    Qt::Test
    kmm_payeeidentifier
    kmm_mymoney
    kmm_testutils
)
