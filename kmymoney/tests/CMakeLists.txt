include(ECMAddTests)

file(GLOB tests_sources "*-test.cpp")
ecm_add_tests(${tests_sources}
  NAME_PREFIX
    "kmymoney-"
  LINK_LIBRARIES
    Qt5::Core
    Qt5::Test
    kmymoney_common
)

add_library(kmm_testutilities STATIC
  testutilities.cpp
)
target_link_libraries(kmm_testutilities
  PUBLIC
    Qt5::Core
  PRIVATE
    mymoneystoragexml
    xmlstoragehelper
)
