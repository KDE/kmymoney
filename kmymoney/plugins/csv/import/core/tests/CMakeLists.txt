include(ECMAddTests)

add_library(csvimport_testcommon STATIC
  csvimporttestcommon.cpp
)

target_link_libraries(csvimport_testcommon
  PUBLIC
  Qt5::Core
  kmm_mymoney
)

file(GLOB tests_sources "*-test.cpp")
ecm_add_tests(${tests_sources}
  NAME_PREFIX
    "csvimport-"
  LINK_LIBRARIES
    Qt5::Test
    kmm_csvimport_core
    csvimport_testcommon
)
