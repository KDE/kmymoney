set (libinterfaces_a_SOURCES
  kmmimportinterface.cpp kmmstatementinterface.cpp
  kmmviewinterface.cpp
  )

add_library(interfaces STATIC ${libinterfaces_a_SOURCES})
target_link_libraries(interfaces KF5::XmlGui KF5::KDELibs4Support Alkimia::alkimia Qt5::Sql)
