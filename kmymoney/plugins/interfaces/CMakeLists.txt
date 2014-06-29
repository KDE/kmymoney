SET (libinterfaces_a_SOURCES
  kmmimportinterface.cpp kmmstatementinterface.cpp
  kmmviewinterface.cpp
  )

add_library(interfaces STATIC ${libinterfaces_a_SOURCES})
