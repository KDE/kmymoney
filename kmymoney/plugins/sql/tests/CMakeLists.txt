include(ECMAddTests)

set(sqlstoragestatic_SOURCES
  ../mymoneystoragesql.cpp
  ../mymoneydbdef.cpp
  ../mymoneydbdriver.cpp
  )

add_library(sqlstoragestatic STATIC ${sqlstoragestatic_SOURCES} )
kde_target_enable_exceptions(sqlstoragestatic PUBLIC)
target_link_libraries(sqlstoragestatic
PUBLIC
  kmm_plugin
  kmm_widgets
  kmm_utils_platformtools
)

file(GLOB tests_sources "*-test.cpp")
ecm_add_tests(${tests_sources}
  LINK_LIBRARIES
    sqlstoragestatic
    Qt5::Test
    kmm_testutils
    kmm_plugin
    kmm_widgets
    kmm_utils_platformtools
    onlinetask_unavailabletask
)
