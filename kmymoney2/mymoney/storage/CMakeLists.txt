SET(storage_HEADERS
  imymoneystorage.h imymoneyserialize.h imymoneystorageformat.h
  )

SET(storage_a_SOURCES
  imymoneystorageformat.cpp mymoneystoragexml.cpp mymoneystoragedump.cpp
  mymoneyseqaccessmgr.cpp imymoneystorage.cpp imymoneyserialize.cpp
  mymoneystorageanon.cpp 
  #mymoneystoragesql.cpp 
  #mymoneydatabasemgr.cpp
  )

KDE4_ADD_KCFG_FILES(storage_a_SOURCES  ../../kmymoneysettings.kcfgc )

# This library is actually not needed. It is built just for
# convenience, and then linked into kmm_mymoney.
KDE4_ADD_LIBRARY( kmm_storage STATIC ${storage_a_SOURCES} )

########### install files ###############

INSTALL(FILES ${storage_HEADERS} 
	DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel )
