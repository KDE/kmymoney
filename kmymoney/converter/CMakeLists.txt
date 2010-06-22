SET (libconverter_a_SOURCES
	mymoneygncreader.cpp
	mymoneyqifprofile.cpp
	mymoneyqifreader.cpp
  mymoneyqifwriter.cpp
	mymoneystatementreader.cpp
	mymoneytemplate.cpp
	webpricequote.cpp
	)

KMM_CREATE_LINKS( mymoneytemplate.h )

KDE4_ADD_LIBRARY(converter STATIC ${libconverter_a_SOURCES})

# we rely on some dialogs to be generated
ADD_DEPENDENCIES(converter dialogs)

########### install files ###############

INSTALL(FILES mymoneytemplate.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)

############## tests ####################
IF( KDE4_BUILD_TESTS )

  set( convertertest_SRCS convertertest.cpp ../reports/reportstestcommon.cpp )

  kde4_add_unit_test( convertertest TESTNAME kmymoney-convertertest ${convertertest_SRCS} )

  target_link_libraries( convertertest converter kmymoney_base kmymoney_common kmm_storage ${QT_AND_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY} )

ENDIF( KDE4_BUILD_TESTS )

