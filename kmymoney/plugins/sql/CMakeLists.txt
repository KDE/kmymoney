configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sqlstorage.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/sqlstorage.json @ONLY)

set(sqlstorage_SOURCES
  sqlstorage.cpp
  mymoneystoragesql.cpp
  mymoneydbdef.cpp
  mymoneydbdriver.cpp
  kselectdatabasedlg.cpp
  kgeneratesqldlg.cpp
  )

if (ENABLE_SQLTRACER)
  list(APPEND sqlstorage_SOURCES
    kmmsqlquery.cpp
  )
endif()

ki18n_wrap_ui(sqlstorage_SOURCES kselectdatabasedlg.ui kgeneratesqldlg.ui)

qt5_add_resources(sqlstorage_SOURCES sqlstorage.qrc)

kmymoney_add_plugin(sqlstorage SOURCES ${sqlstorage_SOURCES})

target_link_libraries(sqlstorage
  PUBLIC
    kmm_plugin
    kmm_widgets
  PRIVATE
    Qt::Sql
    kmm_utils_platformtools
    xmlstoragehelper
)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
