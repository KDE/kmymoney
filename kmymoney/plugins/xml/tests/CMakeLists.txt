include(ECMAddTests)

set(mymoneystoragexml_SOURCES
  ../mymoneystoragexml.cpp
  ../mymoneystoragenames.cpp
  )

add_library(mymoneystoragexml STATIC ${mymoneystoragexml_SOURCES})
target_link_libraries(mymoneystoragexml
  PUBLIC
    Qt5::Xml
    KF5::I18n
    kmm_mymoney
  PRIVATE
    xmlstoragehelper
)

file(GLOB tests_sources "*-test.cpp")
ecm_add_tests(${tests_sources}
  LINK_LIBRARIES
    Qt5::Test
    mymoneystoragexml
)
