set(xmlstoragehelper_SOURCES
  xmlstoragehelper.cpp
  )

add_library(xmlstoragehelper STATIC ${xmlstoragehelper_SOURCES})

target_link_libraries(xmlstoragehelper
  PRIVATE
    Qt5::Xml
    kmm_mymoney
)
