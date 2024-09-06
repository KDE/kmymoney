add_executable(xea2kmt xea2kmt.cpp)
target_link_libraries(xea2kmt kmm_mymoney)

add_executable(kmmreport-export
  kmmreport-export.cpp
)
target_include_directories(kmmreport-export
  PRIVATE
    ../plugins/xml
)
target_link_libraries(kmmreport-export
  PRIVATE
    kmm_mymoney
    kmm_testutilities
    reports
    xmlstorage_static
    Qt::Core
)
