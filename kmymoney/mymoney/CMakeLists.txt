add_subdirectory( storage )
add_subdirectory( payeeidentifier )

########### next target ###############

set(kmm_mymoney_LIB_SRCS
  mymoneymoney.cpp mymoneyfinancialcalculator.cpp
  mymoneytransactionfilter.cpp
  mymoneyfile.cpp mymoneykeyvaluecontainer.cpp
  mymoneyobject.cpp
  mymoneypayeeidentifiercontainer.cpp
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
  onlinejob.cpp
  onlinejobadministration.cpp
  onlinejobmessage.cpp
  onlinejobfolder.cpp
  mymoneycontact.cpp
  payeeidentifiermodel.cpp
  )


set(mymoney_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/kmm_mymoney_export.h
  mymoneyobject.h mymoneyaccount.h mymoneycategory.h mymoneyexception.h
  mymoneyfile.h mymoneyfinancialcalculator.h mymoneyinstitution.h
  mymoneyinvesttransaction.h mymoneykeyvaluecontainer.h mymoneymoney.h
  mymoneypayee.h mymoneytag.h mymoneyprice.h mymoneyreport.h
  mymoneyschedule.h mymoneysecurity.h mymoneysplit.h mymoneystatement.h
  mymoneytransactionfilter.h mymoneytransaction.h
  mymoneyutils.h mymoneybudget.h mymoneyobjectcontainer.h mymoneyforecast.h
  imymoneyprocessingcalendar.h
  mymoneyfile.h
  mymoneyunittestable.h imymoneyprocessingcalendar.h
  mymoneypayeeidentifiercontainer.h
  onlinejob.h
  onlinejobtyped.h
  onlinejobmessage.h
  onlinejobfolder.h
  )

kde4_add_library(kmm_mymoney SHARED ${kmm_mymoney_LIB_SRCS})
generate_export_header(kmm_mymoney BASE_NAME kmm_mymoney)

target_link_libraries(kmm_mymoney LINK_PUBLIC
                      ${KDE4_KDEUI_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTXML_LIBRARY}
                      ${QT_QTCORE_LIBRARY}
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
                      ${KDEPIMLIBS_AKONADI_CONTACT_LIBS}
                      ${LIBALKIMIA_LIBRARIES}
                      kmm_storage
                      payeeidentifier_iban_bic
#                     onlinetask_interfaces
                      LINK_PRIVATE
                      kmm_payeeidentifier_loader
                      kmm_payeeidentifier
                      payeeidentifier_iban_bic
                      payeeidentifier_nationalAccount
                      kmm_settings
                      onlinetask_unavailabletask
)

set_target_properties(kmm_mymoney PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

########### install files ###############

install(TARGETS kmm_mymoney
  ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES ${mymoney_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)

############## tests ####################
if( KDE4_BUILD_TESTS )

  set(target_link_libraries_test
    ${QT_QTGUI_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTTEST_LIBRARY}
    ${LIBALKIMIA_LIBRARIES})

  # MyMoneyAccount
  set( mymoneyaccounttest_SRCS mymoneyaccounttest.cpp )

  kde4_add_unit_test( mymoneyaccounttest TESTNAME kmymoney-mymoneyaccounttest ${mymoneyaccounttest_SRCS} )
  add_dependencies( mymoneyaccounttest kmm_mymoney )

  target_link_libraries( mymoneyaccounttest kmm_mymoney ${target_link_libraries_test})

  # MyMoneyException
  set( mymoneyexceptiontest_SRCS mymoneyexceptiontest.cpp )

  kde4_add_unit_test( mymoneyexceptiontest TESTNAME kmymoney-mymoneyexceptiontest ${mymoneyexceptiontest_SRCS} )
  add_dependencies( mymoneyexceptiontest kmm_mymoney )

  target_link_libraries( mymoneyexceptiontest kmm_mymoney ${target_link_libraries_test})

  # MyMoneyFile
  set( mymoneyfiletest_SRCS mymoneyfiletest.cpp mymoneytestutils.cpp )

  kde4_add_unit_test( mymoneyfiletest TESTNAME kmymoney-mymoneyfiletest ${mymoneyfiletest_SRCS} )
  add_dependencies( mymoneyfiletest kmm_mymoney kmm_storage )

  target_link_libraries( mymoneyfiletest kmm_storage kmm_mymoney ${target_link_libraries_test} )

  # MyMoneyFinancialCalculatorTest
  set( mymoneyfinancialcalculatortest_SRCS mymoneyfinancialcalculatortest.cpp mymoneytestutils.cpp )

  kde4_add_unit_test( mymoneyfinancialcalculatortest TESTNAME kmymoney-mymoneyfinancialcalculatortest ${mymoneyfinancialcalculatortest_SRCS} )
  add_dependencies( mymoneyfinancialcalculatortest kmm_mymoney kmm_storage )

  target_link_libraries( mymoneyfinancialcalculatortest kmm_mymoney kmm_storage ${target_link_libraries_test})

  # MyMoneyForecast
  set( mymoneyforecasttest_SRCS mymoneyforecasttest.cpp mymoneytestutils.cpp ../reports/reportstestcommon.cpp )

  kde4_add_unit_test( mymoneyforecasttest TESTNAME kmymoney-mymoneyforecasttest ${mymoneyforecasttest_SRCS} )
  add_dependencies( mymoneyforecasttest kmm_mymoney kmm_storage )

  target_link_libraries( mymoneyforecasttest kmm_storage kmm_mymoney ${target_link_libraries_test})

  # MyMoneyInstitution
  set( mymoneyinstitutiontest_SRCS mymoneyinstitutiontest.cpp )

  kde4_add_unit_test( mymoneyinstitutiontest TESTNAME kmymoney-mymoneyinstitutiontest ${mymoneyinstitutiontest_SRCS} )
  add_dependencies( mymoneyinstitutiontest kmm_mymoney )

  target_link_libraries( mymoneyinstitutiontest kmm_mymoney ${target_link_libraries_test})

  # MyMoneyKeyValueContainer
  set( mymoneykeyvaluecontainertest_SRCS mymoneykeyvaluecontainertest.cpp )

  kde4_add_unit_test( mymoneykeyvaluecontainertest TESTNAME kmymoney-mymoneykeyvaluecontainertest ${mymoneykeyvaluecontainertest_SRCS} )
  add_dependencies( mymoneykeyvaluecontainertest kmm_mymoney )

  target_link_libraries( mymoneykeyvaluecontainertest kmm_mymoney ${target_link_libraries_test})

  # MyMoneyMoney
  set( mymoneymoneytest_SRCS mymoneymoneytest.cpp )

  kde4_add_unit_test( mymoneymoneytest TESTNAME kmymoney-mymoneymoneytest ${mymoneymoneytest_SRCS} )
  add_dependencies( mymoneymoneytest kmm_mymoney)

  target_link_libraries( mymoneymoneytest kmm_mymoney ${target_link_libraries_test})

  # MyMoneyObject
  set( mymoneyobjecttest_SRCS mymoneyobjecttest.cpp )

  kde4_add_unit_test( mymoneyobjecttest TESTNAME kmymoney-mymoneyobjecttest ${mymoneyobjecttest_SRCS} )
  add_dependencies( mymoneyobjecttest kmm_mymoney )

  target_link_libraries( mymoneyobjecttest kmm_mymoney ${target_link_libraries_test})

  # onlineJob
  set( onlinejobtest_SRCS onlinejobtest.cpp )

  kde4_add_unit_test( onlinejobtest TESTNAME kmymoney-onlinejobtest ${onlinejobtest_SRCS} )
  ADD_DEPENDENCIES( onlinejobtest kmm_mymoney )

  target_link_libraries( onlinejobtest kmm_mymoney onlinetask_interfaces ${target_link_libraries_test})

  # MyMoneyPayee
  set( mymoneypayeetest_SRCS mymoneypayeetest.cpp )

  kde4_add_unit_test( mymoneypayeetest TESTNAME kmymoney-mymoneypayeetest ${mymoneypayeetest_SRCS} )
  add_dependencies( mymoneypayeetest kmm_mymoney )

  target_link_libraries( mymoneypayeetest kmm_mymoney ${target_link_libraries_test})

  # MyMoneyTag
  set( mymoneytagtest_SRCS mymoneytagtest.cpp )

  kde4_add_unit_test( mymoneytagtest TESTNAME kmymoney-mymoneytagtest ${mymoneytagtest_SRCS} )
  add_dependencies( mymoneytagtest kmm_mymoney )

  target_link_libraries( mymoneytagtest kmm_mymoney ${target_link_libraries_test})

  # MyMoneyPrice
  set( mymoneypricetest_SRCS mymoneypricetest.cpp )

  kde4_add_unit_test( mymoneypricetest TESTNAME kmymoney-mymoneypricetest ${mymoneypricetest_SRCS} )
  add_dependencies( mymoneypricetest kmm_mymoney )

  target_link_libraries( mymoneypricetest kmm_mymoney ${target_link_libraries_test})

  # MyMoneySchedule
  set( mymoneyscheduletest_SRCS mymoneyscheduletest.cpp )

  kde4_add_unit_test( mymoneyscheduletest TESTNAME kmymoney-mymoneyscheduletest ${mymoneyscheduletest_SRCS} )
  add_dependencies( mymoneyscheduletest kmm_mymoney )

  target_link_libraries( mymoneyscheduletest kmm_mymoney ${target_link_libraries_test})

  # MyMoneySecurity
  set( mymoneysecuritytest_SRCS mymoneysecuritytest.cpp )

  kde4_add_unit_test( mymoneysecuritytest TESTNAME kmymoney-mymoneysecuritytest ${mymoneysecuritytest_SRCS} )
  add_dependencies( mymoneysecuritytest kmm_mymoney )

  target_link_libraries( mymoneysecuritytest kmm_mymoney ${target_link_libraries_test})

  # MyMoneySplit
  set( mymoneysplittest_SRCS mymoneysplittest.cpp )

  kde4_add_unit_test( mymoneysplittest TESTNAME kmymoney-mymoneysplittest ${mymoneysplittest_SRCS} )
  add_dependencies( mymoneysplittest kmm_mymoney )

  target_link_libraries( mymoneysplittest kmm_mymoney ${target_link_libraries_test})

  # MyMoneyTransaction
  set( mymoneytransactiontest_SRCS mymoneytransactiontest.cpp mymoneytestutils.cpp )

  kde4_add_unit_test( mymoneytransactiontest TESTNAME kmymoney-mymoneytransactiontest ${mymoneytransactiontest_SRCS} )
  add_dependencies( mymoneytransactiontest kmm_mymoney )

  target_link_libraries( mymoneytransactiontest kmm_mymoney ${target_link_libraries_test})

  # MyMoneyBudget
  set( mymoneybudgettest_SRCS mymoneybudgettest.cpp mymoneytestutils.cpp )

  kde4_add_unit_test( mymoneybudgettest TESTNAME kmymoney-mymoneybudgettest ${mymoneybudgettest_SRCS} )
  add_dependencies( mymoneybudgettest kmm_mymoney )

  target_link_libraries( mymoneybudgettest kmm_mymoney ${target_link_libraries_test})

  # MyMoneyBalanceCache
  set( mymoneybalancecachetest_SRCS mymoneybalancecachetest.cpp mymoneytestutils.cpp )

  kde4_add_unit_test( mymoneybalancecachetest TESTNAME kmymoney-mymoneybalancecachetest ${mymoneybalancecachetest_SRCS} )
  add_dependencies( mymoneybalancecachetest kmm_mymoney )

  target_link_libraries( mymoneybalancecachetest kmm_mymoney ${target_link_libraries_test})

  # OnlineJobAdministration
  set( onlinejobtestadministration_SRCS onlinejobadministrationtest.cpp )

  kde4_add_unit_test( onlinejobadministrationtest TESTNAME kmymoney-onlinejobadministrationtest ${onlinejobtestadministration_SRCS} )
  add_dependencies( onlinejobadministrationtest kmm_mymoney )

  target_link_libraries( onlinejobadministrationtest kmm_mymoney kmm_storage onlinetask_interfaces ${target_link_libraries_test})

  # onlineJobTyped
  set( onlinejobtypedtest_SRCS onlinejobtypedtest.cpp )

  kde4_add_unit_test( onlinejobtypedtest TESTNAME kmymoney-onlinejobtypedtest ${onlinejobtypedtest_SRCS} )
  #ADD_DEPENDENCIES( onlinejobknowntasktest  )

  target_link_libraries( onlinejobtypedtest kmm_mymoney onlinetask_interfaces ${target_link_libraries_test})

  # mymoneytransactionfiltertest
  set( mymoneytransactionfiltertest_SRCS mymoneytransactionfiltertest.cpp )
  kde4_add_unit_test( mymoneytransactionfiltertest TESTNAME kmymoney-mymoneytransactionfiltertest ${mymoneytransactionfiltertest_SRCS} )
  add_dependencies( mymoneytransactionfiltertest kmm_mymoney )

  target_link_libraries( mymoneytransactionfiltertest kmm_mymoney ${target_link_libraries_test})

endif( KDE4_BUILD_TESTS )
