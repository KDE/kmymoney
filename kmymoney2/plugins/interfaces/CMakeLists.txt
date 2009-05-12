INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET (libinterfaces_a_SOURCES
  kmmimportinterface.cpp kmmstatementinterface.cpp
  #kmmviewinterface.cpp
  )

KDE4_ADD_LIBRARY(interfaces STATIC ${libinterfaces_a_SOURCES})
