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
  Qt::Sql
  KF5::CoreAddons
  kmm_plugin
  kmm_widgets
  kmm_utils_platformtools
  onlinetask_interfaces
  xmlstoragehelper
)

file(GLOB tests_sources "*-test.cpp")

# for now don't build database test as it will be replaced
message("mymoneydatabasemgr-test.cpp disabled in ${CMAKE_CURRENT_SOURCE_DIR}")
list(REMOVE_ITEM tests_sources ${CMAKE_CURRENT_SOURCE_DIR}/mymoneydatabasemgr-test.cpp)

ecm_add_tests(${tests_sources}
  LINK_LIBRARIES
    sqlstoragestatic
    Qt::Test
    kmm_testutils
    kmm_plugin
    kmm_widgets
    kmm_utils_platformtools
    kmm_mymoney
    xmlstoragehelper
)
