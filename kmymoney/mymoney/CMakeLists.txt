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
  mymoneyinvesttransaction.cpp mymoneyutils.cpp
  mymoneysecurity.cpp mymoneytransaction.cpp mymoneyschedule.cpp
  mymoneypayee.cpp mymoneytracer.cpp
  mymoneytag.cpp
  mymoneycategory.cpp
  mymoneycostcenter.cpp
  mymoneyaccount.cpp mymoneyaccountloan.cpp
  mymoneyreport.cpp mymoneystatement.cpp mymoneyprice.cpp mymoneybudget.cpp
  mymoneyforecast.cpp
  mymoneytemplate.cpp
  mymoneybalancecache.cpp
  onlinejob.cpp
  onlinejobadministration.cpp
  onlinejobmessage.cpp
  onlinejobfolder.cpp
  mymoneycontact.cpp
  payeeidentifiermodel.cpp
  statusmodel.cpp
  splitmodel.cpp
  )

  # storage_a_SOURCES cannot be set in storage directory on MS Windows
  # because, while building kmm_storage, linker reports many undefined symbols
  # which are in fact available in kmm_mymoney
  set(storage_a_SOURCES
    ./storage/imymoneystorageformat.cpp
    ./storage/mymoneymodelbase.cpp
    ./storage/payeesmodel.cpp
    ./storage/costcentermodel.cpp
    ./storage/schedulesmodel.cpp
    ./storage/tagsmodel.cpp
    ./storage/securitiesmodel.cpp
    ./storage/budgetsmodel.cpp
    ./storage/accountsmodel.cpp
    ./storage/institutionsmodel.cpp
    ./storage/journalmodel.cpp
    ./storage/pricemodel.cpp
    ./storage/parametersmodel.cpp
    ./storage/onlinejobsmodel.cpp
    ./storage/reportsmodel.cpp
    ./storage/reconciliationmodel.cpp
    ./storage/specialdatesmodel.cpp
    ./storage/schedulesjournalmodel.cpp
    ./storage/templatesmodel.cpp
    )

  set(onlineTask_a_SOURCES
  ./onlinetasks/sepa/sepaonlinetransferimpl.cpp
  ./onlinetasks/sepa/sepaonlinetransfer.cpp
  ./onlinetasks/unavailabletask/tasks/unavailabletask.cpp
  )

  # NOTE: this payeeIdentifier and its cmake file cannot be used as a template
  # This payeeIdentifier must be linked with payeeIdentifierLoader because it
  # is a fallback if something failed during loading of plugins (for xml files only)
  set(payeeidentifier_a_SOURCES
  ./payeeidentifier/ibanbic/ibanbic.cpp
  ./payeeidentifier/nationalaccount/nationalaccount.cpp
  ./payeeidentifier/unavailableplugin/unavailableplugin.cpp
  )

list(APPEND storage_a_SOURCES $<$<OR:$<CONFIG:Debug>,$<CONFIG:DebugFull>,$<CONFIG:DebugKMM>>: ./storage/mymoneystoragedump.cpp>)
list(APPEND storage_a_SOURCES $<$<BOOL:${USE_MODELTEST}>:modeltest.cpp>)
list(APPEND kmm_mymoney_LIB_SRCS ${storage_a_SOURCES})
list(APPEND kmm_mymoney_LIB_SRCS ${onlineTask_a_SOURCES})
list(APPEND kmm_mymoney_LIB_SRCS ${payeeidentifier_a_SOURCES})

set(mymoney_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/kmm_mymoney_export.h
  mymoneyobject.h mymoneyaccount.h mymoneycategory.h mymoneyexception.h
  mymoneyfile.h mymoneyfinancialcalculator.h mymoneyinstitution.h
  mymoneyinvesttransaction.h mymoneykeyvaluecontainer.h mymoneymoney.h
  mymoneypayee.h mymoneytag.h mymoneyprice.h mymoneyreport.h
  mymoneyschedule.h mymoneysecurity.h mymoneysplit.h mymoneystatement.h
  mymoneytransactionfilter.h mymoneytransaction.h
  mymoneyutils.h mymoneybudget.h mymoneyforecast.h
  imymoneyprocessingcalendar.h
  mymoneycostcenter.h
  mymoneyenums.h
  mymoneytemplate.h
  mymoneyunittestable.h
  mymoneypayeeidentifiercontainer.h
  onlinejob.h
  onlinejobtyped.h
  onlinejobmessage.h
  onlinejobfolder.h
  )

add_library(kmm_mymoney SHARED ${kmm_mymoney_LIB_SRCS})

if (USE_MODELTEST)
  target_compile_definitions(kmm_mymoney PRIVATE -DKMM_MODELTEST)
endif()

generate_export_header(kmm_mymoney BASE_NAME kmm_mymoney)

target_link_libraries(kmm_mymoney
                      PUBLIC
                      kmm_icons
                      Qt5::Xml
                      Qt5::Core
                      Qt5::Gui
                      KF5::Service
                      KF5::I18n
                      KF5::ItemModels
                      Alkimia::alkimia
                      kmm_payeeidentifier
                      kmm_plugin
                      # TODO: fix this
                      KF5::XmlGui
                      PRIVATE
                      onlinetask_interfaces
                      $<$<BOOL:${USE_MODELTEST}>:Qt5::Test>
)

if(ENABLE_ADDRESSBOOK)
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
