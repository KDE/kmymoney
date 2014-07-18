SET (libinterfaces_a_SOURCES
  kmmimportinterface.cpp kmmstatementinterface.cpp
  kmmviewinterface.cpp
  )

add_library(interfaces STATIC ${libinterfaces_a_SOURCES})
target_link_libraries(interfaces ${GMP_LIBRARIES} Alkimia::alkimia)
