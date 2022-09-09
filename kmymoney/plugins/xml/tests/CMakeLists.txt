include(ECMAddTests)

set(mymoneystoragexml_SOURCES
  ../mymoneystoragenames.cpp
  ../mymoneyxmlreader.cpp
  )

add_library(mymoneystoragexml STATIC ${mymoneystoragexml_SOURCES})
target_link_libraries(mymoneystoragexml
  PUBLIC
    Qt::Xml
    KF5::I18n
    onlinetask_interfaces
    kmm_mymoney
  PRIVATE
    xmlstoragehelper
)

file(GLOB tests_sources "*-test.cpp")
ecm_add_tests(${tests_sources}
  LINK_LIBRARIES
    Qt::Test
    mymoneystoragexml
    kmm_testutilities
)
