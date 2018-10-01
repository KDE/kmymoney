set(storage_HEADERS
  imymoneystorage.h imymoneyserialize.h imymoneystorageformat.h
  kmymoneystorageplugin.h
  databasestoreableobject.h
  )

set(storage_a_SOURCES
  imymoneystorageformat.cpp mymoneystoragexml.cpp mymoneystoragedump.cpp
  mymoneyseqaccessmgr.cpp imymoneystorage.cpp imymoneyserialize.cpp
  mymoneystorageanon.cpp
  mymoneystoragesql.cpp
  mymoneydatabasemgr.cpp
  mymoneydbdef.cpp
  mymoneydbdriver.cpp
  kmymoneystorageplugin.cpp
  )

# This library is actually not needed. It is built just for
# convenience, and then linked into kmm_mymoney.
kde4_add_library( kmm_storage STATIC ${storage_a_SOURCES} )
target_link_libraries( kmm_storage ${LIBALKIMIA_LIBRARIES} )
if( WIN32 )
  target_compile_definitions(kmm_storage PUBLIC KMM_MYMONEY_EXPORT=)
endif()

########### install files ###############

install(FILES ${storage_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel )

install(FILES kmymoney-sqlstorageplugin.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR}
)

############## tests ####################
if( KDE4_BUILD_TESTS )

  set(target_link_libraries_test
    ${LIBALKIMIA_LIBRARIES}
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
  add_dependencies( mymoneydatabasemgrtest kmymoney_base kmymoney_common kmm_storage )

  target_link_libraries( mymoneydatabasemgrtest kmymoney_base kmymoney_common kmm_storage onlinetask_interfaces ${target_link_libraries_test} )

  # MyMoneyMap
  set( mymoneymaptest_SRCS mymoneymaptest.cpp ../mymoneytestutils.cpp )

  kde4_add_unit_test( mymoneymaptest TESTNAME kmymoney-mymoneymaptest ${mymoneymaptest_SRCS} )
  add_dependencies( mymoneymaptest kmymoney_base kmymoney_common kmm_storage )

  target_link_libraries( mymoneymaptest kmymoney_base kmymoney_common kmm_storage ${target_link_libraries_test} )

  # MyMoneySeqAccessMgr
  set( mymoneyseqaccessmgrtest_SRCS mymoneyseqaccessmgrtest.cpp ../mymoneytestutils.cpp )

  kde4_add_unit_test( mymoneyseqaccessmgrtest TESTNAME kmymoney-mymoneyseqaccessmgrtest ${mymoneyseqaccessmgrtest_SRCS} )
  add_dependencies( mymoneyseqaccessmgrtest kmymoney_base kmymoney_common kmm_storage )

  target_link_libraries( mymoneyseqaccessmgrtest kmymoney_base kmymoney_common kmm_storage onlinetask_interfaces ${target_link_libraries_test} )

endif( KDE4_BUILD_TESTS )
