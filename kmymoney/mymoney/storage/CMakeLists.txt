if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

set(storage_HEADERS
  imymoneystorageformat.h
  mymoneystoragemgr.h
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
  )

########### install files ###############

install(FILES ${storage_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel )
