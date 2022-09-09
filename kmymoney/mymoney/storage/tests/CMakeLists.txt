include(ECMAddTests)

file(GLOB tests_sources "*-test.cpp")

# @todo for now don't use storagemgr-test which will be replaced
message("mymoneystoragemgr-test.cpp disabled in ${CMAKE_CURRENT_SOURCE_DIR}")
list(REMOVE_ITEM tests_sources ${CMAKE_CURRENT_SOURCE_DIR}/mymoneystoragemgr-test.cpp)

ecm_add_tests(${tests_sources}
  LINK_LIBRARIES
    Qt::Core
    Qt::Test
    Qt::Xml
    Qt::Gui
    kmm_mymoney
    kmm_testutils
    kmm_utils_platformtools
    onlinetask_interfaces
)
