# qsql_sqlite between Qt versions aren't necessarily compatible and can fail to compile
# qsql_sqlite is copied from https://github.com/sjemens/qsqlcipher-qt5
if (Qt5Core_VERSION VERSION_GREATER 5.10.99)
  set (QSQLITE_VERSION 5.11) #untested

elseif (Qt5Core_VERSION VERSION_GREATER 5.9.99)
  set (QSQLITE_VERSION 5.10) #tested

elseif (Qt5Core_VERSION VERSION_GREATER 5.7.99)
  set (QSQLITE_VERSION 5.9) #untested

else()
  set (QSQLITE_VERSION 5.7) #untested

endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/QSQLite/${QSQLITE_VERSION}/qsql_sqlite_p.h ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/QSQLite/${QSQLITE_VERSION}/qsql_sqlite.cpp ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

set ( qsqlcipher_SOURCES
  smain.cpp
  qsql_sqlite.cpp
)

set ( qsqlcipher_HEADERS
  qsql_sqlite_p.h
)

kcoreaddons_add_plugin(qsqlcipher
                       SOURCES ${qsqlcipher_SOURCES}
                       JSON "${CMAKE_CURRENT_SOURCE_DIR}/sqlcipher.json"
                       INSTALL_NAMESPACE "sqldrivers")

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(qsqlcipher
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

#otherwise "fatal error: 'sqlite3.h' file not found"
if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR CMAKE_SYSTEM_NAME MATCHES "Windows")
  target_include_directories(qsqlcipher
    PRIVATE
      ${SQLCIPHER_INCLUDE_DIRS})
endif()

target_compile_definitions(qsqlcipher
  PRIVATE
  QT_PLUGIN # fails to build on MS Windows without this
  SQLITE_HAS_CODEC
  SQLITE_TEMP_STORE=2)

target_link_libraries(qsqlcipher
  PRIVATE
    sqlcipher
    Qt5::SqlPrivate
)

if(BUILD_TESTING)
 add_subdirectory(tests)
endif()
