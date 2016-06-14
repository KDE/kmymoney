add_subdirectory( tests )

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

target_link_libraries(converter PUBLIC KF5::KDELibs4Support Qt5::Sql Qt5::Xml Alkimia::alkimia)

# we rely on some dialogs to be generated
add_dependencies(converter dialogs)

########### install files ###############

install(FILES mymoneytemplate.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)

