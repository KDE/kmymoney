set (libinterfaces_a_SOURCES
  kmmimportinterface.cpp kmmstatementinterface.cpp
  kmmviewinterface.cpp
  )

kde4_add_library(interfaces STATIC ${libinterfaces_a_SOURCES})
