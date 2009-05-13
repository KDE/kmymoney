INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET (libconverter_a_SOURCES 
	#mymoneygncreader.cpp 
	#mymoneyqifprofile.cpp 
	mymoneyqifreader.cpp

  	mymoneyqifwriter.cpp 
	#mymoneystatementreader.cpp 
	#mymoneytemplate.cpp
  webpricequote.cpp
	)


KMM_CREATE_LINKS( mymoneytemplate.h )

KDE4_ADD_LIBRARY(converter STATIC ${libconverter_a_SOURCES})

########### install files ###############

INSTALL(FILES mymoneytemplate.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)
