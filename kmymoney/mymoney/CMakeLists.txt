ADD_SUBDIRECTORY( storage )

########### next target ###############

SET(kmm_mymoney_LIB_SRCS
  mymoneymoney.cpp mymoneyfinancialcalculator.cpp 
  mymoneytransactionfilter.cpp
  mymoneyobject.cpp mymoneykeyvaluecontainer.cpp 
  mymoneysplit.cpp mymoneyinstitution.cpp
  mymoneyexception.cpp mymoneyinvesttransaction.cpp mymoneyutils.cpp
  mymoneysecurity.cpp mymoneytransaction.cpp mymoneyscheduled.cpp
  mymoneypayee.cpp 
  mymoneyfile.cpp 
  mymoneycategory.cpp 
  mymoneyaccount.cpp
  mymoneyreport.cpp mymoneystatement.cpp mymoneyprice.cpp mymoneybudget.cpp
  mymoneyobjectcontainer.cpp 
  mymoneyforecast.cpp
  )


SET(mymoney_HEADERS kmm_mymoney_export.h
  mymoneyobject.h mymoneyaccount.h mymoneycategory.h mymoneyexception.h
  mymoneyfile.h mymoneyfinancialcalculator.h mymoneyinstitution.h
  mymoneyinvesttransaction.h mymoneykeyvaluecontainer.h mymoneymoney.h
  mymoneypayee.h mymoneyprice.h mymoneyreport.h
  mymoneyscheduled.h mymoneysecurity.h mymoneysplit.h mymoneystatement.h
  mymoneytransactionfilter.h mymoneytransaction.h
  mymoneyutils.h mymoneybudget.h mymoneyobjectcontainer.h mymoneyforecast.h
  imymoneyprocessingcalendar.h
  )

KMM_CREATE_LINKS( ${mymoney_HEADERS} )

KDE4_ADD_LIBRARY(kmm_mymoney SHARED 
	${kmm_mymoney_LIB_SRCS})

TARGET_LINK_LIBRARIES(kmm_mymoney ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTCORE_LIBRARY} ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} kmm_storage)

SET_TARGET_PROPERTIES(kmm_mymoney PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

########### install files ###############

INSTALL(TARGETS kmm_mymoney
	${INSTALL_TARGETS_DEFAULT_ARGS} )

INSTALL(FILES ${mymoney_HEADERS}
	DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)

############## tests ####################
IF( KDE4_BUILD_TESTS )

  # MyMoneyAccount
  set( mymoneyaccounttest_SRCS mymoneyaccounttest.cpp )

  kde4_add_unit_test( mymoneyaccounttest TESTNAME kmymoney-mymoneyaccounttest ${mymoneyaccounttest_SRCS} )
  ADD_DEPENDENCIES( mymoneyaccounttest kmm_mymoney )

  target_link_libraries( mymoneyaccounttest kmm_mymoney ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} )

  # MyMoneyException
  set( mymoneyexceptiontest_SRCS mymoneyexceptiontest.cpp )

  kde4_add_unit_test( mymoneyexceptiontest TESTNAME kmymoney-mymoneyexceptiontest ${mymoneyexceptiontest_SRCS} )
  ADD_DEPENDENCIES( mymoneyexceptiontest kmm_mymoney )

  target_link_libraries( mymoneyexceptiontest kmm_mymoney ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} )

  # MyMoneyFile
  set( mymoneyfiletest_SRCS mymoneyfiletest.cpp mymoneytestutils.cpp )

  kde4_add_unit_test( mymoneyfiletest TESTNAME kmymoney-mymoneyfiletest ${mymoneyfiletest_SRCS} )
  ADD_DEPENDENCIES( mymoneyfiletest kmm_mymoney kmm_storage )

  target_link_libraries( mymoneyfiletest kmm_storage kmm_mymoney ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} )

  # MyMoneyFinancialCalculatorTest
  set( mymoneyfinancialcalculatortest_SRCS mymoneyfinancialcalculatortest.cpp mymoneytestutils.cpp )

  kde4_add_unit_test( mymoneyfinancialcalculatortest TESTNAME kmymoney-mymoneyfinancialcalculatortest ${mymoneyfinancialcalculatortest_SRCS} )
  ADD_DEPENDENCIES( mymoneyfinancialcalculatortest kmm_mymoney kmm_storage )

  target_link_libraries( mymoneyfinancialcalculatortest kmm_mymoney kmm_storage ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} )

  # MyMoneyForecast
  set( mymoneyforecasttest_SRCS mymoneyforecasttest.cpp mymoneytestutils.cpp ../reports/reportstestcommon.cpp )

  kde4_add_unit_test( mymoneyforecasttest TESTNAME kmymoney-mymoneyforecasttest ${mymoneyforecasttest_SRCS} )
  ADD_DEPENDENCIES( mymoneyforecasttest kmm_mymoney kmm_storage )

  target_link_libraries( mymoneyforecasttest kmm_storage kmm_mymoney ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} )

  # MyMoneyInstitution
  set( mymoneyinstitutiontest_SRCS mymoneyinstitutiontest.cpp )

  kde4_add_unit_test( mymoneyinstitutiontest TESTNAME kmymoney-mymoneyinstitutiontest ${mymoneyinstitutiontest_SRCS} )
  ADD_DEPENDENCIES( mymoneyinstitutiontest kmm_mymoney )

  target_link_libraries( mymoneyinstitutiontest kmm_mymoney ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} )

  # MyMoneyKeyValueContainer
  set( mymoneykeyvaluecontainertest_SRCS mymoneykeyvaluecontainertest.cpp )

  kde4_add_unit_test( mymoneykeyvaluecontainertest TESTNAME kmymoney-mymoneykeyvaluecontainertest ${mymoneykeyvaluecontainertest_SRCS} )
  ADD_DEPENDENCIES( mymoneykeyvaluecontainertest kmm_mymoney )

  target_link_libraries( mymoneykeyvaluecontainertest kmm_mymoney ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} )

  # MyMoneyMoney
  set( mymoneymoneytest_SRCS mymoneymoneytest.cpp )

  kde4_add_unit_test( mymoneymoneytest TESTNAME kmymoney-mymoneymoneytest ${mymoneymoneytest_SRCS} )
  ADD_DEPENDENCIES( mymoneymoneytest kmm_mymoney )

  target_link_libraries( mymoneymoneytest kmm_mymoney ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} )

  # MyMoneyObject
  set( mymoneyobjecttest_SRCS mymoneyobjecttest.cpp )

  kde4_add_unit_test( mymoneyobjecttest TESTNAME kmymoney-mymoneyobjecttest ${mymoneyobjecttest_SRCS} )
  ADD_DEPENDENCIES( mymoneyobjecttest kmm_mymoney )

  target_link_libraries( mymoneyobjecttest kmm_mymoney ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} )

  # MyMoneyPayee
  set( mymoneypayeetest_SRCS mymoneypayeetest.cpp )

  kde4_add_unit_test( mymoneypayeetest TESTNAME kmymoney-mymoneypayeetest ${mymoneypayeetest_SRCS} )
  ADD_DEPENDENCIES( mymoneypayeetest kmm_mymoney )

  target_link_libraries( mymoneypayeetest kmm_mymoney ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} )

  # MyMoneyPrice
  set( mymoneypricetest_SRCS mymoneypricetest.cpp )

  kde4_add_unit_test( mymoneypricetest TESTNAME kmymoney-mymoneypricetest ${mymoneypricetest_SRCS} )
  ADD_DEPENDENCIES( mymoneypricetest kmm_mymoney )

  target_link_libraries( mymoneypricetest kmm_mymoney ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} )

  # MyMoneySchedule
  set( mymoneyscheduletest_SRCS mymoneyscheduletest.cpp )

  kde4_add_unit_test( mymoneyscheduletest TESTNAME kmymoney-mymoneyscheduletest ${mymoneyscheduletest_SRCS} )
  ADD_DEPENDENCIES( mymoneyscheduletest kmm_mymoney )

  target_link_libraries( mymoneyscheduletest kmm_mymoney ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS})

  # MyMoneySecurity
  set( mymoneysecuritytest_SRCS mymoneysecuritytest.cpp )

  kde4_add_unit_test( mymoneysecuritytest TESTNAME kmymoney-mymoneysecuritytest ${mymoneysecuritytest_SRCS} )
  ADD_DEPENDENCIES( mymoneysecuritytest kmm_mymoney )

  target_link_libraries( mymoneysecuritytest kmm_mymoney ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} )

  # MyMoneySplit
  set( mymoneysplittest_SRCS mymoneysplittest.cpp )

  kde4_add_unit_test( mymoneysplittest TESTNAME kmymoney-mymoneysplittest ${mymoneysplittest_SRCS} )
  ADD_DEPENDENCIES( mymoneysplittest kmm_mymoney )

  target_link_libraries( mymoneysplittest kmm_mymoney ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} )

  # MyMoneyTransaction
  set( mymoneytransactiontest_SRCS mymoneytransactiontest.cpp mymoneytestutils.cpp )

  kde4_add_unit_test( mymoneytransactiontest TESTNAME kmymoney-mymoneytransactiontest ${mymoneytransactiontest_SRCS} )
  ADD_DEPENDENCIES( mymoneytransactiontest kmm_mymoney )

  target_link_libraries( mymoneytransactiontest kmm_mymoney ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} )

  # MyMoneyBudget
  set( mymoneybudgettest_SRCS mymoneybudgettest.cpp mymoneytestutils.cpp )

  kde4_add_unit_test( mymoneybudgettest TESTNAME kmymoney-mymoneybudgettest ${mymoneybudgettest_SRCS} )
  add_dependencies( mymoneybudgettest kmm_mymoney )

  target_link_libraries( mymoneybudgettest kmm_mymoney ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} )

ENDIF( KDE4_BUILD_TESTS )
