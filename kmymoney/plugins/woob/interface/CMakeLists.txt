set(woob_interface_SOURCES
        woobinterface.cpp
    )

include_directories(${Python3_INCLUDE_DIRS})

qt5_add_resources(woob_interface_SOURCES woobinterface.qrc)

add_library(woob_interface STATIC ${woob_interface_SOURCES})
target_link_libraries (
  woob_interface
  PUBLIC
  Qt5::Core
  Alkimia::alkimia
  PRIVATE
  ${Python3_LIBRARIES}
  )