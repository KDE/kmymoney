include(ECMAddTests)

file(GLOB tests_sources "*-test.cpp")
ecm_add_tests(${tests_sources}
  NAME_PREFIX
    "csvimport-"
  LINK_LIBRARIES
    Qt5::Core
    Qt5::Test
    kmm_csvimport_static
)
