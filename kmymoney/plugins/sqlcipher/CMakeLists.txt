# qsql_sqlite between Qt versions aren't necessarily compatible and can fail to compile
# qsql_sqlite is copied from https://github.com/sjemens/qsqlcipher-qt5

set ( qsqlcipher_SOURCES
  smain.cpp
  qsql_sqlite.cpp
)

if (BUILD_STATIC_PLUGINS)
    add_library(qsqlcipher STATIC ${qsqlcipher_SOURCES})
    target_compile_definitions(qsqlcipher PRIVATE QT_STATICPLUGIN)
else()
    kcoreaddons_add_plugin(qsqlcipher
                        SOURCES ${qsqlcipher_SOURCES}
                        INSTALL_NAMESPACE "sqldrivers")
endif()

#otherwise "fatal error: 'sqlite3.h' file not found"
if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR CMAKE_SYSTEM_NAME MATCHES "Windows" OR CMAKE_VERSION VERSION_LESS 3.5.2)
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
    PkgConfig::SQLCIPHER
    Qt5::SqlPrivate
)

if(BUILD_TESTING)
 add_subdirectory(tests)
endif()
