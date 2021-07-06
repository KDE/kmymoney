set (libinterfaces_a_SOURCES
  kmmappinterface.cpp
  kmmimportinterface.cpp
  kmmstatementinterface.cpp
  kmmviewinterface.cpp
  )

add_library(interfaces STATIC ${libinterfaces_a_SOURCES})


# TODO: fix dependencies
target_link_libraries(interfaces
    KF5::Service
    KF5::XmlGui
    Alkimia::alkimia
    Qt5::Xml
    KF5::I18n
    onlinetask_interfaces
)
