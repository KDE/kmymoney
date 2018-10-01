set (libconverter_a_SOURCES
  mymoneygncreader.cpp
  mymoneyqifprofile.cpp
  mymoneyqifreader.cpp
  mymoneyqifwriter.cpp
  mymoneystatementreader.cpp
  mymoneytemplate.cpp
  transactionmatchfinder.cpp
  existingtransactionmatchfinder.cpp
  scheduledtransactionmatchfinder.cpp
)

kde4_add_library(converter STATIC ${libconverter_a_SOURCES})
target_link_libraries(converter ${LIBALKIMIA_LIBRARIES})

# we rely on some dialogs to be generated
add_dependencies(converter dialogs)

########### install files ###############

install(FILES mymoneytemplate.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)

############## tests ####################
if( KDE4_BUILD_TESTS )

  set( matchfindertest_SRCS matchfindertest.cpp )
  kde4_add_unit_test( matchfindertest TESTNAME kmymoney-matchfindertest ${matchfindertest_SRCS} )
  target_link_libraries( matchfindertest converter widgets kmymoney_common dialogs widgets kmm_settings kmm_storage ${KDE4_KUTILS_LIBRARY} ${QT_QTTEST_LIBRARY} )

endif( KDE4_BUILD_TESTS )
