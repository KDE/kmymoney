include(ECMAddTests)


file(GLOB tests_sources "*-test.cpp")
ecm_add_tests(${tests_sources}
  LINK_LIBRARIES
    Qt5::Core
    Qt5::Test
    Qt5::Xml
    Qt5::Gui
    kmm_mymoney
    kmm_testutils
    kmm_utils_platformtools
    onlinetask_interfaces
)
