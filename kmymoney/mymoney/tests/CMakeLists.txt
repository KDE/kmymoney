include(ECMAddTests)

add_library(kmm_testutils STATIC mymoneytestutils.cpp)
target_link_libraries(kmm_testutils
  PUBLIC
    Qt5::Core
    kmm_mymoney
)

file(GLOB tests_sources "*-test.cpp")
ecm_add_tests(${tests_sources}
  LINK_LIBRARIES
    Qt5::Core
    Qt5::Test
    Qt5::Xml
    Qt5::Gui
    kmm_mymoney
    kmm_testutils
    reports_testcommon
    onlinetask_interfaces
)
