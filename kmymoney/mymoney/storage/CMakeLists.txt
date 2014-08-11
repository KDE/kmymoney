set(storage_HEADERS
  imymoneystorage.h imymoneyserialize.h imymoneystorageformat.h
  )

set(storage_a_SOURCES
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
add_library( kmm_storage STATIC ${storage_a_SOURCES} )
kde_target_enable_exceptions(kmm_storage PUBLIC)

target_link_libraries( kmm_storage PUBLIC KF5::I18n Qt5::Core Qt5::Xml Qt5::Sql Qt5::Gui Alkimia::alkimia)

########### install files ###############

install(FILES ${storage_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel )

############## tests ####################
if( KDE4_BUILD_TESTS )

  set(target_link_libraries_test
    Alkimia::alkimia
    KF5::KIOWidgets
    Qt5::Gui
    Qt5::Xml
    Qt5::Test
    Qt5::Sql
    Qt5::Core)

  # MyMoneyDatabaseMgr
  set( mymoneydatabasemgrtest_SRCS mymoneydatabasemgrtest.cpp ../mymoneytestutils.cpp )

  kde4_add_unit_test( mymoneydatabasemgrtest TESTNAME kmymoney-mymoneydatabasemgrtest ${mymoneydatabasemgrtest_SRCS} )
  add_dependencies( mymoneydatabasemgrtest kmymoney_base kmymoney_common kmm_storage )

  target_link_libraries( mymoneydatabasemgrtest kmymoney_base kmymoney_common kmm_storage ${target_link_libraries_test} )

  # MyMoneyMap
  set( mymoneymaptest_SRCS mymoneymaptest.cpp ../mymoneytestutils.cpp )

  kde4_add_unit_test( mymoneymaptest TESTNAME kmymoney-mymoneymaptest ${mymoneymaptest_SRCS} )
  add_dependencies( mymoneymaptest kmymoney_base kmymoney_common kmm_storage )

  target_link_libraries( mymoneymaptest kmymoney_base kmymoney_common kmm_storage ${target_link_libraries_test} )

  # MyMoneySeqAccessMgr
  set( mymoneyseqaccessmgrtest_SRCS mymoneyseqaccessmgrtest.cpp ../mymoneytestutils.cpp )

  kde4_add_unit_test( mymoneyseqaccessmgrtest TESTNAME kmymoney-mymoneyseqaccessmgrtest ${mymoneyseqaccessmgrtest_SRCS} )
  add_dependencies( mymoneyseqaccessmgrtest kmymoney_base kmymoney_common kmm_storage )

  target_link_libraries( mymoneyseqaccessmgrtest kmymoney_base kmymoney_common kmm_storage ${target_link_libraries_test} )

endif( KDE4_BUILD_TESTS )
