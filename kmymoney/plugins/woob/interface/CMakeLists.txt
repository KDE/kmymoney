set(woob_interface_SOURCES
        woobinterface.cpp
    )

include_directories(${Python3_INCLUDE_DIRS})

qt_add_resources(woob_interface_SOURCES woobinterface.qrc)

add_library(woob_interface STATIC ${woob_interface_SOURCES})
target_link_libraries (
  woob_interface
  PUBLIC
  Qt::Core
  Alkimia::alkimia
  PRIVATE
  ${Python3_LIBRARIES}
  )
