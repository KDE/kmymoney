#    Alkimia::alkimia
#    KF5::KIOWidgets
#    Qt5::Gui
#    Qt5::Sql

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
    reports_testcommon
    onlinetask_unavailabletask
    kmm_utils_platformtools
)
