include(ECMAddTests)

file(GLOB tests_sources "*-test.cpp")
ecm_add_tests(${tests_sources}
  NAME_PREFIX
    "ibanandbic-"
  LINK_LIBRARIES
    Qt::Test
    kmm_mymoney
)
