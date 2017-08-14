include(ECMAddTests)

if(BUILD_TESTING)
  add_library(reports_testcommon STATIC
    reportstestcommon.cpp
  )
  target_link_libraries(reports_testcommon
    PUBLIC
    Qt5::Core
    reports
  )
endif()

file(GLOB tests_sources "*-test.cpp")
ecm_add_tests(${tests_sources}
  NAME_PREFIX
    "reports-"
  LINK_LIBRARIES
    Qt5::Core
    Qt5::Test
    kmm_mymoney
    reports_testcommon
)

if(BUILD_TESTING)
  add_executable(reports-chart-test chart-test.cpp)
  target_link_libraries(reports-chart-test reports Qt5::Core Qt5::Gui)
endif()
