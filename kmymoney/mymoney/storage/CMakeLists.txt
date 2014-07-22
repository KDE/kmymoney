SET(storage_HEADERS
  imymoneystorage.h imymoneyserialize.h imymoneystorageformat.h
  )

SET(storage_a_SOURCES
  imymoneystorageformat.cpp mymoneystoragexml.cpp mymoneystoragedump.cpp
  mymoneyseqaccessmgr.cpp imymoneystorage.cpp imymoneyserialize.cpp
  mymoneystorageanon.cpp
  mymoneystoragesql.cpp
  mymoneydatabasemgr.cpp
  mymoneydbdef.cpp
  mymoneydbdriver.cpp
  )

# This library is actually not needed. It is built just for
# convenience, and then linked into kmm_mymoney.
KDE4_ADD_LIBRARY( kmm_storage STATIC ${storage_a_SOURCES} )

########### install files ###############

INSTALL(FILES ${storage_HEADERS} 
	DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel )

############## tests ####################
IF( KDE4_BUILD_TESTS )

  set(target_link_libraries_test 
	  ${LIBALKIMIA_LIBRARY}
	  ${KDE4_KDECORE_LIBS}
	  ${KDE4_KIO_LIBS}
	  ${QT_QTGUI_LIBRARY}
	  ${QT_QTXML_LIBRARY}
	  ${QT_QTTEST_LIBRARY}
	  ${QT_QTSQL_LIBRARY}
	  ${QT_QTCORE_LIBRARY})

  # MyMoneyDatabaseMgr
  set( mymoneydatabasemgrtest_SRCS mymoneydatabasemgrtest.cpp ../mymoneytestutils.cpp )

  kde4_add_unit_test( mymoneydatabasemgrtest TESTNAME kmymoney-mymoneydatabasemgrtest ${mymoneydatabasemgrtest_SRCS} )
  ADD_DEPENDENCIES( mymoneydatabasemgrtest kmymoney_base kmymoney_common kmm_storage )

  target_link_libraries( mymoneydatabasemgrtest kmymoney_base kmymoney_common kmm_storage ${target_link_libraries_test} )

  # MyMoneyMap
  set( mymoneymaptest_SRCS mymoneymaptest.cpp ../mymoneytestutils.cpp )

  kde4_add_unit_test( mymoneymaptest TESTNAME kmymoney-mymoneymaptest ${mymoneymaptest_SRCS} )
  ADD_DEPENDENCIES( mymoneymaptest kmymoney_base kmymoney_common kmm_storage )

  target_link_libraries( mymoneymaptest kmymoney_base kmymoney_common kmm_storage ${target_link_libraries_test} )

  # MyMoneySeqAccessMgr
  set( mymoneyseqaccessmgrtest_SRCS mymoneyseqaccessmgrtest.cpp ../mymoneytestutils.cpp )

  kde4_add_unit_test( mymoneyseqaccessmgrtest TESTNAME kmymoney-mymoneyseqaccessmgrtest ${mymoneyseqaccessmgrtest_SRCS} )
  ADD_DEPENDENCIES( mymoneyseqaccessmgrtest kmymoney_base kmymoney_common kmm_storage )

  target_link_libraries( mymoneyseqaccessmgrtest kmymoney_base kmymoney_common kmm_storage ${target_link_libraries_test} )

ENDIF( KDE4_BUILD_TESTS )
