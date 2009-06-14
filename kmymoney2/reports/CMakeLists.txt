INCLUDE_DIRECTORIES( ${KMyMoney_SOURCE_DIR}/libkdchart)

SET (libreports_a_SOURCES
  #kreportchartview.cpp 
  listtable.cpp 
  objectinfotable.cpp 
  pivotgrid.cpp
  pivottable.cpp 
  querytable.cpp reportaccount.cpp
)

KDE4_ADD_LIBRARY(reports STATIC ${libreports_a_SOURCES})
