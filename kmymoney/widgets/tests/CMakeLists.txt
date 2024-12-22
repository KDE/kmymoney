include(ECMAddTests)

file(GLOB tests_sources "*-test.cpp")
kmm_add_tests(${tests_sources}
  LINK_LIBRARIES
    Qt::Gui
    Qt::Core
    Qt::Test
    kmm_base_widgets
    kmm_testutils
)
