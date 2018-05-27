if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

set(storage_HEADERS
  imymoneystorageformat.h
  mymoneystoragemgr.h
  )

########### install files ###############

install(FILES ${storage_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel )
