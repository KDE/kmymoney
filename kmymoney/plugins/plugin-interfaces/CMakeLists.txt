set (libinterfaces_a_SOURCES
  kmmappinterface.cpp
  kmmimportinterface.cpp
  kmmstatementinterface.cpp
  kmmviewinterface.cpp
  )

add_library(interfaces STATIC ${libinterfaces_a_SOURCES})


# TODO: fix dependencies
target_link_libraries(interfaces
    KF${QT_MAJOR_VERSION}::Service
    KF${QT_MAJOR_VERSION}::XmlGui
    Alkimia::alkimia
    Qt::Xml
    KF${QT_MAJOR_VERSION}::I18n
    onlinetask_interfaces
)
