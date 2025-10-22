set(mymoneystoragexml_SOURCES
  ../mymoneystoragenames.cpp
  ../mymoneyxmlreader.cpp
  ../mymoneyxmlwriter.cpp
  )

add_library(mymoneystoragexml STATIC ${mymoneystoragexml_SOURCES})
target_link_libraries(mymoneystoragexml
  PUBLIC
    Qt::Xml
    KF${QT_MAJOR_VERSION}::I18n
    onlinetask_interfaces
    kmm_mymoney
  PRIVATE
    xmlstoragehelper
)

add_definitions(-DCMAKE_CURRENT_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\" -DCMAKE_CURRENT_BINARY_DIR=\"${CMAKE_CURRENT_BINARY_DIR}\")
file(GLOB tests_sources "*-test.cpp")
kmm_add_tests(${tests_sources}
  LINK_LIBRARIES
    Qt::Test
    mymoneystoragexml
    kmm_testutils
    kmm_testutilities
)
