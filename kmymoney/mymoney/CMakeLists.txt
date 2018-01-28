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
  mymoneypayee.cpp mymoneytracer.cpp
  mymoneytag.cpp
  mymoneycategory.cpp
  mymoneycostcenter.cpp
  mymoneyaccount.cpp mymoneyaccountloan.cpp
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
  mymoneycostcenter.h
  mymoneyunittestable.h
  mymoneypayeeidentifiercontainer.h
  onlinejob.h
  onlinejobtyped.h
  onlinejobmessage.h
  onlinejobfolder.h
  )

add_library(kmm_mymoney SHARED ${kmm_mymoney_LIB_SRCS})

target_compile_features(kmm_mymoney PUBLIC cxx_nullptr PRIVATE cxx_decltype cxx_lambdas cxx_constexpr cxx_range_for)

generate_export_header(kmm_mymoney BASE_NAME kmm_mymoney)

target_link_libraries(kmm_mymoney
                      PUBLIC
                      kmm_icons
                      Qt5::Xml
                      Qt5::Core
                      KF5::Service
                      Alkimia::alkimia
                      kmm_storage
                      kmm_payeeidentifier_loader
                      kmm_payeeidentifier
                      payeeidentifier_iban_bic
                      payeeidentifier_nationalAccount
                      # TODO: fix this
                      KF5::XmlGui
                      PRIVATE
                      onlinetask_unavailabletask
)

if(KMM_ADDRESSBOOK_FOUND)
target_link_libraries(kmm_mymoney PUBLIC KF5::IdentityManagement KF5::AkonadiCore KF5::Contacts)
endif()

set_target_properties(kmm_mymoney PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

########### install files ###############

install(TARGETS kmm_mymoney
  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES ${mymoney_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)

############## tests ####################

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
