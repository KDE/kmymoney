if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

set(storage_HEADERS
  imymoneystorageformat.h
  mymoneymodelbase.h
  mymoneymodel.h
  payeesmodel.h
  costcentermodel.h
  schedulesmodel.h
  tagsmodel.h
  securitiesmodel.h
  budgetsmodel.h
  accountsmodel.h
  institutionsmodel.h
  journalmodel.h
  pricemodel.h
  parametersmodel.h
  onlinejobsmodel.h
  reportsmodel.h
  reconciliationmodel.h
  )

########### install files ###############

install(FILES ${storage_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kmymoney COMPONENT Devel )
