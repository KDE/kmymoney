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
    mysqlquery.cpp
  )
endif()

ki18n_wrap_ui(sqlstorage_SOURCES kselectdatabasedlg.ui kgeneratesqldlg.ui)

kcoreaddons_add_plugin(sqlstorage
                       SOURCES ${sqlstorage_SOURCES}
                       JSON "${CMAKE_CURRENT_BINARY_DIR}/sqlstorage.json"
                       INSTALL_NAMESPACE "kmymoney")

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(sqlstorage
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

target_link_libraries(sqlstorage
  PUBLIC
    kmm_plugin
    kmm_widgets
  PRIVATE
    Qt5::Sql
    kmm_utils_platformtools
    xmlstoragehelper
)

install(FILES sqlstorage.rc
        DESTINATION "${KXMLGUI_INSTALL_DIR}/sqlstorage")

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
