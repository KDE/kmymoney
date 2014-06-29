INCLUDE_DIRECTORIES( ${KMyMoney_SOURCE_DIR}/libkdchart/include)

SET (libreports_a_SOURCES
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

ADD_DEPENDENCIES(reports kmm_config)

############## tests ####################
IF( KDE4_BUILD_TESTS )

  # Pivot Grid
  set( pivotgridtest_SRCS pivotgridtest.cpp reportstestcommon.cpp )

  kde4_add_unit_test( pivotgridtest TESTNAME kmymoney-pivotgridtest ${pivotgridtest_SRCS} )
  ADD_DEPENDENCIES( pivotgridtest reports kmymoney_base kmymoney_common kmm_storage )

  target_link_libraries( pivotgridtest reports kmymoney_base kmymoney_common kmm_storage KF5::KHtml KF5::Archive KF5::Holidays Qt5::Gui Qt5::Xml Qt5::Core Qt5::Test )

  # Pivot Table
  set( pivottabletest_SRCS pivottabletest.cpp reportstestcommon.cpp ../mymoney/mymoneytestutils.cpp )

  kde4_add_unit_test( pivottabletest TESTNAME kmymoney-pivottabletest ${pivottabletest_SRCS} )

  ADD_DEPENDENCIES( pivottabletest dialogs widgets reports settings kmymoney_base kmymoney_common kmm_kdchart kmm_storage kmm_config )

  target_link_libraries( pivottabletest reports settings kmymoney_base kmymoney_common dialogs widgets kmm_kdchart kmm_storage kmm_config KF5::KHtml KF5::Archive KF5::Holidays Qt5::Gui Qt5::Xml Qt5::Core Qt5::Test )

  # Query Table
  set( querytabletest_SRCS querytabletest.cpp reportstestcommon.cpp ../mymoney/mymoneytestutils.cpp )

  kde4_add_unit_test( querytabletest TESTNAME kmymoney-querytabletest ${querytabletest_SRCS} )
  ADD_DEPENDENCIES( querytabletest dialogs widgets reports settings kmymoney_base kmymoney_common kmm_storage kmm_config )

  target_link_libraries( querytabletest reports kmymoney_base kmymoney_common dialogs widgets kmm_storage kmm_config KF5::KHtml KF5::Archive KF5::Holidays Qt5::Gui Qt5::Xml Qt5::Core Qt5::Test )

ENDIF( KDE4_BUILD_TESTS )
