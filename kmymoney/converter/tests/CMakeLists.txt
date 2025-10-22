file(GLOB tests_sources "*-test.cpp")
kmm_add_tests(${tests_sources}
  LINK_LIBRARIES
    Qt::Core
    Qt::Test
    kmm_testutils
    kmm_testutilities
    converter
)
