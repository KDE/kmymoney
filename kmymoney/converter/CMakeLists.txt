SET (libconverter_a_SOURCES
	mymoneygncreader.cpp
	mymoneyqifprofile.cpp
	mymoneyqifreader.cpp
  mymoneyqifwriter.cpp
	mymoneystatementreader.cpp
	mymoneytemplate.cpp
	webpricequote.cpp
	)

KDE4_ADD_LIBRARY(converter STATIC ${libconverter_a_SOURCES})

########### install files ###############

INSTALL(FILES mymoneytemplate.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)
