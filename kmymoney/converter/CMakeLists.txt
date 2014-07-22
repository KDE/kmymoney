set (libconverter_a_SOURCES
  mymoneygncreader.cpp
  mymoneyqifprofile.cpp
  mymoneyqifreader.cpp
  mymoneyqifwriter.cpp
  mymoneystatementreader.cpp
  mymoneytemplate.cpp
  webpricequote.cpp
  transactionmatchfinder.cpp
  existingtransactionmatchfinder.cpp
  scheduledtransactionmatchfinder.cpp
)

add_library(converter STATIC ${libconverter_a_SOURCES})

target_link_libraries(converter PUBLIC Qt5::Sql Alkimia::alkimia)

# we rely on some dialogs to be generated
add_dependencies(converter dialogs)

########### install files ###############

install(FILES mymoneytemplate.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)

############## tests ####################
if( KDE4_BUILD_TESTS )
  set( convertertest_SRCS convertertest.cpp ../reports/reportstestcommon.cpp )
  kde4_add_unit_test( convertertest TESTNAME kmymoney-convertertest ${convertertest_SRCS} )
  target_link_libraries( convertertest converter kmymoney_base kmymoney_common kmm_storage ${QT_AND_KDECORE_LIBS} KF5::KIOWidgets Qt5::Test )

  set( matchfindertest_SRCS matchfindertest.cpp )
  kde4_add_unit_test( matchfindertest TESTNAME kmymoney-matchfindertest ${matchfindertest_SRCS} )
  target_link_libraries( matchfindertest converter widgets kmymoney_common dialogs widgets kmm_config kmm_storage ${KDE4_KUTILS_LIBRARY} Qt5::Test )

endif( KDE4_BUILD_TESTS )
