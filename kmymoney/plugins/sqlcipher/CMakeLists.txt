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
)
if (TARGET Qt::SqlPrivate)
  target_link_libraries(qsqlcipher PRIVATE Qt::SqlPrivate)
endif()
target_link_libraries(qsqlcipher PRIVATE Qt::Sql)

# On Windows the qsqlcipher plugin imports sqlite3.dll: SQLCipher is built as a
# drop-in replacement named sqlite3.dll. If no sqlite3.dll is present next to the
# executable that loads the plugin, QPluginLoader fails to instantiate the plugin
# and QSqlDatabase reports "can not load requested driver" even though the plugin
# shows up in QSqlDatabase::drivers(). So we deploy the SQLCipher runtime DLL into
# the runtime output directory whenever the plugin is built (not only for tests).
#
# pkg-config gives us the import-library dirs (SQLCIPHER_LIBRARY_DIRS); in a
# Craft/typical layout the runtime DLL sits in the sibling bin/ directory.
if(WIN32 AND MSVC AND NOT BUILD_STATIC_PLUGINS)
  find_file(SQLCIPHER_RUNTIME_DLL
    NAMES sqlite3.dll libsqlite3.dll sqlcipher.dll libsqlcipher.dll
    HINTS
      ${SQLCIPHER_LIBRARY_DIRS}
      ${SQLCIPHER_PREFIX}
    PATH_SUFFIXES bin ../bin
    NO_DEFAULT_PATH)

  if(SQLCIPHER_RUNTIME_DLL)
    message(STATUS "qsqlcipher: deploying SQLCipher runtime DLL: ${SQLCIPHER_RUNTIME_DLL}")
    add_custom_command(TARGET qsqlcipher POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E echo "==== deploying SQLCipher runtime (sqlite3.dll) ===="
      COMMAND ${CMAKE_COMMAND} -E echo "source: ${SQLCIPHER_RUNTIME_DLL}"
      COMMAND ${CMAKE_COMMAND} -E echo "dest:   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/sqlite3.dll"
      COMMAND ${CMAKE_COMMAND} -E copy
              "${SQLCIPHER_RUNTIME_DLL}"
              "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/sqlite3.dll"
      COMMAND ${CMAKE_COMMAND} -E echo "==================================================="
      VERBATIM)
  else()
    message(WARNING
      "qsqlcipher: could not locate the SQLCipher runtime sqlite3.dll "
      "(searched SQLCIPHER_LIBRARY_DIRS=${SQLCIPHER_LIBRARY_DIRS}). "
      "The QSQLCIPHER driver will fail to load on Windows.")
  endif()
endif()

if(BUILD_TESTING)
 add_subdirectory(tests)
endif()
