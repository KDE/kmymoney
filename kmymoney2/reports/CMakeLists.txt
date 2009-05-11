INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )
INCLUDE_DIRECTORIES( ${KMyMoney_SOURCE_DIR}/libkdchart)

SET (libreports_a_SOURCES
  kreportchartview.cpp listtable.cpp objectinfotable.cpp pivotgrid.cpp
  pivottable.cpp querytable.cpp reportaccount.cpp
  )

#KDE3_AUTOMOC(${libreports_a_SOURCES})

KDE4_ADD_LIBRARY(reports STATIC ${libreports_a_SOURCES})
