include(ECMAddTests)

add_library(kmm_testutils STATIC mymoneytestutils.cpp)
target_link_libraries(kmm_testutils
  PUBLIC
    Qt::Core
    kmm_mymoney
)

file(GLOB tests_sources "*-test.cpp")
ecm_add_tests(${tests_sources}
  LINK_LIBRARIES
    Qt::Core
    Qt::Test
    Qt::Xml
    Qt::Gui
    kmm_mymoney
    kmm_testutils
    kmm_testutilities
    onlinetask_interfaces
)
