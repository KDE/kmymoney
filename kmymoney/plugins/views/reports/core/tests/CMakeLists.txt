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

# CI: workaround for
# The following tests FAILED:
#   31 - reports-chart-test (Subprocess aborted)
# FATAL : ChartTest::createChart() ASSERT: "topLeftIndex.row() <= bottomRightIndex.row()"
# in file /builds/sysadmin/ci-management/kdiagram/src/KChart/Cartesian/KChartCartesianDiagramDataCompressor_p.cpp, line 238
if(BUILD_WITH_QT6)
  list(APPEND debug_env
    "QT_LOGGING_RULES=qt.*.debug=true"
    "QT_DEBUG_PLUGINS=1"
  )
endif()

set(tests_gui_sources
  chart-test.cpp
)

ecm_add_tests(${tests_gui_sources}
  NAME_PREFIX
    "reports-"
  ENV
    "QT_QPA_PLATFORM=offscreen"
    ${debug_env}
  GUI
  LINK_LIBRARIES
    reports
    Qt::Core
    Qt::Gui
    Qt::Test
)

add_subdirectory(data)

