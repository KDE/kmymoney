set( germanonlinetransfertest_SRCS germanonlinetransfertest.cpp )

kde4_add_unit_test( germanonlinetransfertest TESTNAME kmymoney-germanOnlineTransferTest ${germanonlinetransfertest_SRCS} )

add_dependencies( germanonlinetransfertest kmm_mymoney )
FIND_PACKAGE(KDE4 4.6.0 REQUIRED)

target_link_libraries( germanonlinetransfertest
  konlinetasks_national
  onlinetask_interfaces
  kmm_mymoney
  ${KDE4_KDEUI_LIBS}
  ${QT_QTCORE_LIBRARIES}
  ${QT_QTGUI_LIBRARIES}
  ${QT_QTXML_LIBRARY}
  ${QT_QTTEST_LIBRARY}
  ${GMP_LIBRARIES}
  ${LIBALKIMIA_LIBRARY}
)
