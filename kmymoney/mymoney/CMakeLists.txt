ADD_SUBDIRECTORY( storage )
ADD_SUBDIRECTORY( payeeidentifier )

# TODO move this to a findKtoblzcheck.cmake and use find_package() instead
find_library( KTOBLZCHECK_LIBRARY NAMES libktoblzcheck ktoblzcheck
    DOC "German account number and bank code check http://ktoblzcheck.sourceforge.net/")
message(STATUS "Found ktoblzcheck at " ${KTOBLZCHECK_LIBRARY})


########### next target ###############

SET(kmm_mymoney_LIB_SRCS
  mymoneymoney.cpp mymoneyfinancialcalculator.cpp
  mymoneytransactionfilter.cpp
  mymoneyfile.cpp mymoneykeyvaluecontainer.cpp
  mymoneyobject.cpp
  mymoneysplit.cpp mymoneyinstitution.cpp
  mymoneyexception.cpp mymoneyinvesttransaction.cpp mymoneyutils.cpp
  mymoneysecurity.cpp mymoneytransaction.cpp mymoneyschedule.cpp
  mymoneypayee.cpp
  mymoneytag.cpp
  mymoneycategory.cpp
  mymoneyaccount.cpp
  mymoneyreport.cpp mymoneystatement.cpp mymoneyprice.cpp mymoneybudget.cpp
  mymoneyobjectcontainer.cpp
  mymoneyforecast.cpp
  mymoneybalancecache.cpp
  bankaccountidentifier.cpp
  germanaccountidentifier.cpp
  swiftaccountidentifier.cpp
  onlinejob.cpp
  onlinejobadministration.cpp
  onlinejobmessage.cpp
  onlinejobfolder.cpp
  payeeidentifier/payeeidentifierloader.cpp
  payeeidentifier/payeeidentifier.cpp # TODO: this is not clean, but I needed a target where this file is in quickly
  )


SET(mymoney_HEADERS kmm_mymoney_export.h
  mymoneyobject.h
  mymoneyaccount.h mymoneycategory.h mymoneyexception.h
  mymoneyfinancialcalculator.h mymoneyinstitution.h
  mymoneyinvesttransaction.h mymoneykeyvaluecontainer.h mymoneymoney.h
  mymoneypayee.h mymoneytag.h mymoneyprice.h mymoneyreport.h
  mymoneyschedule.h mymoneysecurity.h mymoneysplit.h mymoneystatement.h
  mymoneytransactionfilter.h mymoneytransaction.h
  mymoneyutils.h mymoneybudget.h mymoneyobjectcontainer.h mymoneyforecast.h
  imymoneyprocessingcalendar.h
  mymoneyfile.h
  onlinejob.h
  accountidentifier.h
  bankaccountidentifier.h
  swiftaccountidentifier.h
  onlinejob.h
  onlinejobtyped.h
  onlinejobmessage.h
  onlinejobfolder.h
  )

KDE4_ADD_LIBRARY(kmm_mymoney SHARED 
	${kmm_mymoney_LIB_SRCS})

TARGET_LINK_LIBRARIES(kmm_mymoney
                      ${KDE4_KDEUI_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTXML_LIBRARY}
                      ${QT_QTCORE_LIBRARY}
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${GMP_LIBRARIES}
                      ${LIBALKIMIA_LIBRARY}
                      ${KTOBLZCHECK_LIBRARY}
                      kmm_storage
#                     onlinetask_interfaces
                      payeeidentifier_iban_bic
                      payeeidentifier_nationalAccount
)
# TODO: Remove payeeidentifier_nationalAccount and payeeidentifier_iban_bic from link list

SET_TARGET_PROPERTIES(kmm_mymoney PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

########### install files ###############

INSTALL(TARGETS kmm_mymoney
	${INSTALL_TARGETS_DEFAULT_ARGS} )

INSTALL(FILES ${mymoney_HEADERS}
	DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)

############## tests ####################
IF( KDE4_BUILD_TESTS )

  set(target_link_libraries_test 
	  ${QT_QTGUI_LIBRARY}
	  ${QT_QTXML_LIBRARY}
 	  ${QT_QTCORE_LIBRARY}
	  ${QT_QTTEST_LIBRARY}
	  ${GMP_LIBRARIES}
          ${LIBALKIMIA_LIBRARY}
)

  # MyMoneyAccount
  set( mymoneyaccounttest_SRCS mymoneyaccounttest.cpp )

  kde4_add_unit_test( mymoneyaccounttest TESTNAME kmymoney-mymoneyaccounttest ${mymoneyaccounttest_SRCS} )
  ADD_DEPENDENCIES( mymoneyaccounttest kmm_mymoney )

  target_link_libraries( mymoneyaccounttest kmm_mymoney ${target_link_libraries_test})

  # MyMoneyException
  set( mymoneyexceptiontest_SRCS mymoneyexceptiontest.cpp )

  kde4_add_unit_test( mymoneyexceptiontest TESTNAME kmymoney-mymoneyexceptiontest ${mymoneyexceptiontest_SRCS} )
  ADD_DEPENDENCIES( mymoneyexceptiontest kmm_mymoney )

  target_link_libraries( mymoneyexceptiontest kmm_mymoney ${target_link_libraries_test})

  # MyMoneyFile
  set( mymoneyfiletest_SRCS mymoneyfiletest.cpp mymoneytestutils.cpp )

  kde4_add_unit_test( mymoneyfiletest TESTNAME kmymoney-mymoneyfiletest ${mymoneyfiletest_SRCS} )
  ADD_DEPENDENCIES( mymoneyfiletest kmm_mymoney kmm_storage )

  target_link_libraries( mymoneyfiletest kmm_storage kmm_mymoney ${target_link_libraries_test} )

  # MyMoneyFinancialCalculatorTest
  set( mymoneyfinancialcalculatortest_SRCS mymoneyfinancialcalculatortest.cpp mymoneytestutils.cpp )

  kde4_add_unit_test( mymoneyfinancialcalculatortest TESTNAME kmymoney-mymoneyfinancialcalculatortest ${mymoneyfinancialcalculatortest_SRCS} )
  ADD_DEPENDENCIES( mymoneyfinancialcalculatortest kmm_mymoney kmm_storage )

  target_link_libraries( mymoneyfinancialcalculatortest kmm_mymoney kmm_storage ${target_link_libraries_test})

  # MyMoneyForecast
  set( mymoneyforecasttest_SRCS mymoneyforecasttest.cpp mymoneytestutils.cpp ../reports/reportstestcommon.cpp )

  kde4_add_unit_test( mymoneyforecasttest TESTNAME kmymoney-mymoneyforecasttest ${mymoneyforecasttest_SRCS} )
  ADD_DEPENDENCIES( mymoneyforecasttest kmm_mymoney kmm_storage )

  target_link_libraries( mymoneyforecasttest kmm_storage kmm_mymoney ${target_link_libraries_test})

  # MyMoneyInstitution
  set( mymoneyinstitutiontest_SRCS mymoneyinstitutiontest.cpp )

  kde4_add_unit_test( mymoneyinstitutiontest TESTNAME kmymoney-mymoneyinstitutiontest ${mymoneyinstitutiontest_SRCS} )
  ADD_DEPENDENCIES( mymoneyinstitutiontest kmm_mymoney )

  target_link_libraries( mymoneyinstitutiontest kmm_mymoney ${target_link_libraries_test})

  # MyMoneyKeyValueContainer
  set( mymoneykeyvaluecontainertest_SRCS mymoneykeyvaluecontainertest.cpp )

  kde4_add_unit_test( mymoneykeyvaluecontainertest TESTNAME kmymoney-mymoneykeyvaluecontainertest ${mymoneykeyvaluecontainertest_SRCS} )
  ADD_DEPENDENCIES( mymoneykeyvaluecontainertest kmm_mymoney )

  target_link_libraries( mymoneykeyvaluecontainertest kmm_mymoney ${target_link_libraries_test})

  # MyMoneyMoney
  set( mymoneymoneytest_SRCS mymoneymoneytest.cpp )

  kde4_add_unit_test( mymoneymoneytest TESTNAME kmymoney-mymoneymoneytest ${mymoneymoneytest_SRCS} )
  ADD_DEPENDENCIES( mymoneymoneytest kmm_mymoney %{${GMP_LIBRARIES})

  target_link_libraries( mymoneymoneytest kmm_mymoney ${target_link_libraries_test})

  # MyMoneyObject
  set( mymoneyobjecttest_SRCS mymoneyobjecttest.cpp )

  kde4_add_unit_test( mymoneyobjecttest TESTNAME kmymoney-mymoneyobjecttest ${mymoneyobjecttest_SRCS} )
  ADD_DEPENDENCIES( mymoneyobjecttest kmm_mymoney )

  target_link_libraries( mymoneyobjecttest kmm_mymoney ${target_link_libraries_test})

  # onlineJob
  set( onlinejobtest_SRCS onlinejobtest.cpp )

  kde4_add_unit_test( onlinejobtest TESTNAME kmymoney-onlinejobtest ${onlinejobtest_SRCS} )
  ADD_DEPENDENCIES( onlinejobtest kmm_mymoney )

  target_link_libraries( onlinejobtest kmm_mymoney onlinetask_interfaces ${target_link_libraries_test})
  
  # MyMoneyPayee
  set( mymoneypayeetest_SRCS mymoneypayeetest.cpp )

  kde4_add_unit_test( mymoneypayeetest TESTNAME kmymoney-mymoneypayeetest ${mymoneypayeetest_SRCS} )
  ADD_DEPENDENCIES( mymoneypayeetest kmm_mymoney )

  target_link_libraries( mymoneypayeetest kmm_mymoney ${target_link_libraries_test})

  # MyMoneyTag
  set( mymoneytagtest_SRCS mymoneytagtest.cpp )

  kde4_add_unit_test( mymoneytagtest TESTNAME kmymoney-mymoneytagtest ${mymoneytagtest_SRCS} )
  ADD_DEPENDENCIES( mymoneytagtest kmm_mymoney )

  target_link_libraries( mymoneytagtest kmm_mymoney ${target_link_libraries_test})

  # MyMoneyPrice
  set( mymoneypricetest_SRCS mymoneypricetest.cpp )

  kde4_add_unit_test( mymoneypricetest TESTNAME kmymoney-mymoneypricetest ${mymoneypricetest_SRCS} )
  ADD_DEPENDENCIES( mymoneypricetest kmm_mymoney )

  target_link_libraries( mymoneypricetest kmm_mymoney ${target_link_libraries_test})

  # MyMoneySchedule
  set( mymoneyscheduletest_SRCS mymoneyscheduletest.cpp )

  kde4_add_unit_test( mymoneyscheduletest TESTNAME kmymoney-mymoneyscheduletest ${mymoneyscheduletest_SRCS} )
  ADD_DEPENDENCIES( mymoneyscheduletest kmm_mymoney )

  target_link_libraries( mymoneyscheduletest kmm_mymoney ${target_link_libraries_test})

  # MyMoneySecurity
  set( mymoneysecuritytest_SRCS mymoneysecuritytest.cpp )

  kde4_add_unit_test( mymoneysecuritytest TESTNAME kmymoney-mymoneysecuritytest ${mymoneysecuritytest_SRCS} )
  ADD_DEPENDENCIES( mymoneysecuritytest kmm_mymoney )

  target_link_libraries( mymoneysecuritytest kmm_mymoney ${target_link_libraries_test})

  # MyMoneySplit
  set( mymoneysplittest_SRCS mymoneysplittest.cpp )

  kde4_add_unit_test( mymoneysplittest TESTNAME kmymoney-mymoneysplittest ${mymoneysplittest_SRCS} )
  ADD_DEPENDENCIES( mymoneysplittest kmm_mymoney )

  target_link_libraries( mymoneysplittest kmm_mymoney ${target_link_libraries_test})

  # MyMoneyTransaction
  set( mymoneytransactiontest_SRCS mymoneytransactiontest.cpp mymoneytestutils.cpp )

  kde4_add_unit_test( mymoneytransactiontest TESTNAME kmymoney-mymoneytransactiontest ${mymoneytransactiontest_SRCS} )
  ADD_DEPENDENCIES( mymoneytransactiontest kmm_mymoney )

  target_link_libraries( mymoneytransactiontest kmm_mymoney ${target_link_libraries_test})

  # MyMoneyBudget
  set( mymoneybudgettest_SRCS mymoneybudgettest.cpp mymoneytestutils.cpp )

  kde4_add_unit_test( mymoneybudgettest TESTNAME kmymoney-mymoneybudgettest ${mymoneybudgettest_SRCS} )
  add_dependencies( mymoneybudgettest kmm_mymoney )

  target_link_libraries( mymoneybudgettest kmm_mymoney ${target_link_libraries_test})

  # MyMoneyBalanceCache
  set( mymoneybalancecachetest_SRCS mymoneybalancecachetest.cpp mymoneytestutils.cpp )

  kde4_add_unit_test( mymoneybalancecachetest TESTNAME kmymoney-mymoneybalancecachetest ${mymoneybalancecachetest_SRCS} )
  ADD_DEPENDENCIES( mymoneybalancecachetest kmm_mymoney )

  target_link_libraries( mymoneybalancecachetest kmm_mymoney ${target_link_libraries_test})

  # OnlineJobAdministration
  set( onlinejobtestadministration_SRCS onlinejobadministrationtest.cpp )

  kde4_add_unit_test( onlinejobadministrationtest TESTNAME kmymoney-onlinejobadministrationtest ${onlinejobtestadministration_SRCS} )
  ADD_DEPENDENCIES( onlinejobadministrationtest kmm_mymoney )

  target_link_libraries( onlinejobadministrationtest kmm_mymoney kmm_storage onlinetask_interfaces ${target_link_libraries_test})

  # onlineJobTyped
  set( onlinejobtypedtest_SRCS onlinejobtypedtest.cpp )

  kde4_add_unit_test( onlinejobtypedtest TESTNAME kmymoney-onlinejobtypedtest ${onlinejobtypedtest_SRCS} )
  #ADD_DEPENDENCIES( onlinejobknowntasktest  )

  target_link_libraries( onlinejobtypedtest kmm_mymoney onlinetask_interfaces ${target_link_libraries_test})


ENDIF( KDE4_BUILD_TESTS )
