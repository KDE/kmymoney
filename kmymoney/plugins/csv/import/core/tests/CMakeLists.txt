add_library(csvimport_testcommon STATIC
  csvimporttestcommon.cpp
)

target_link_libraries(csvimport_testcommon
  PUBLIC
  Qt::Core
  kmm_mymoney
)

file(GLOB tests_sources "*-test.cpp")
kmm_add_tests(${tests_sources}
  NAME_PREFIX
    "csvimport-"
  LINK_LIBRARIES
    Qt::Test
    kmm_csvimportercore
    kmm_testutils
    csvimport_testcommon
)
