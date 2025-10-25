set(tests_sources
  test-pivotgrid.cpp
  test-pivottable.cpp
  test-querytable.cpp
)

add_definitions(-DCMAKE_CURRENT_BINARY_DIR=\"${CMAKE_CURRENT_BINARY_DIR}\")

kmm_add_tests(${tests_sources}
  NAME_PREFIX
    "reports-"
  LINK_LIBRARIES
    Qt::Core
    Qt::Test
    reports
    kmm_mymoney
    kmm_codec
    kmm_testutils
    kmm_testutilities
)

set(tests_gui_sources
  test-chart.cpp
)

# do not run this test on KDE CI, as it is broken for unknown reason
if(NOT "$ENV{KDECI_BUILD}")
    kmm_add_tests(${tests_gui_sources}
      NAME_PREFIX
        "reports-"
      ENVIRONMENT
        "QT_QPA_PLATFORM=offscreen"
      GUI
      LINK_LIBRARIES
        reports
        Qt::Core
        Qt::Gui
        Qt::Test
    )
endif()

add_subdirectory(data)

