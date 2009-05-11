INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET (libinterfaces_a_SOURCES
  kmmimportinterface.cpp kmmstatementinterface.cpp
  kmmviewinterface.cpp
  )

#KDE3_AUTOMOC(${libinterfaces_a_SOURCES})

KDE4_ADD_LIBRARY(interfaces STATIC ${libinterfaces_a_SOURCES})
