include(ECMAddTests)

set(tests_sources
  pivotgrid-test.cpp
  pivottable-test.cpp
  querytable-test.cpp
)

add_definitions(-DCMAKE_CURRENT_BINARY_DIR=\"${CMAKE_CURRENT_BINARY_DIR}\")

ecm_add_tests(${tests_sources}
  ENV
    "LANG=C"
  NAME_PREFIX
    "reports-"
  LINK_LIBRARIES
    Qt::Core
    Qt::Test
    reports
    kmm_mymoney
    kmm_testutils
    kmm_testutilities
)

set(tests_gui_sources
  chart-test.cpp
)

ecm_add_tests(${tests_gui_sources}
  NAME_PREFIX
    "reports-"
  ENV
    "QT_QPA_PLATFORM=offscreen"
  GUI
  LINK_LIBRARIES
    reports
    Qt::Core
    Qt::Gui
    Qt::Test
)

add_subdirectory(data)

