file(GLOB tests_sources "test-*.cpp")
kmm_add_tests(${tests_sources}
  NAME_PREFIX
    "kmymoney-"
  LINK_LIBRARIES
    Qt::Core
    Qt::Test
    kmm_testutils
    kmymoney_common
)

add_library(kmm_testutilities STATIC
  testutilities.cpp
)
target_link_libraries(kmm_testutilities
  PUBLIC
    Qt::Core
  PRIVATE
    mymoneystoragexml
    xmlstoragehelper
)
