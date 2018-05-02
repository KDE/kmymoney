if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

set (libreports_a_SOURCES
  kreportchartview.cpp
  reportaccount.cpp
  listtable.cpp
  objectinfotable.cpp
  pivotgrid.cpp
  pivottable.cpp
  querytable.cpp
  reporttable.cpp
  kbalanceaxis.cpp
)

add_library(reports STATIC ${libreports_a_SOURCES})
target_link_libraries(reports
  PUBLIC
    KChart
    Alkimia::alkimia
    Qt5::Sql
    Qt5::PrintSupport
    kmymoney_common
    kmm_settings
    PRIVATE
    KF5::I18n
)

add_dependencies(reports kmm_settings)
