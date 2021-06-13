set(weboob_interface_SOURCES
    weboobinterface.cpp
    )

include_directories(${Python3_INCLUDE_DIRS})

qt5_add_resources(weboob_interface_SOURCES weboobinterface.qrc)

add_library(weboob_interface STATIC ${weboob_interface_SOURCES})
target_link_libraries (
  weboob_interface
  PUBLIC
  Qt5::Core
  Alkimia::alkimia
  PRIVATE
  ${Python3_LIBRARIES}
  )