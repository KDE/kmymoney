add_subdirectory(tests)

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
    KF5::KDELibs4Support
    Alkimia::alkimia
    Qt5::Sql
    kmymoney_common
)

add_dependencies(reports kmm_config)
