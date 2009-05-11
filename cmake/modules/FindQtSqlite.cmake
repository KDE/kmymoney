# Find whether Qt-Sqlite3 is installed
# And if not, make sure our tarball gets extracted and compiled
#
# Variables:
#  QT_SQLITE_FOUND - system has Qt-Sqlite3
#  QT_SQLITE_FALLBACK - our fallback library is used
#  QSQLITE3_INSTALL_DIR - the install directory for the Qt-Sqlite3 driver

FIND_PACKAGE(Sqlite)

SET(QSQLITE3_INSTALL_DIR "${QT_INSTALL_DIR}/plugins/sqldrivers" CACHE PATH
  "Install directory for our self-compiled qsqlite3 driver.")
MARK_AS_ADVANCED(QSQLITE3_INSTALL_DIR)

# Look for libsqlite3[.lib[64]].so in ${QSQLITE3_INSTALL_DIR}
# NOTE: This is NOT the sqlite3 library /usr/lib/sqlite3.so
FIND_LIBRARY(QT_SQLITE3_LIB NAMES sqlite3.lib64 sqlite3.lib sqlite3
  HINTS ${QSQLITE3_INSTALL_DIR} ${QT_DIR}/plugins/sqldrivers
  NO_DEFAULT_PATH)

if(QT_SQLITE3_LIB)
  message(STATUS "Found Qt-Sqlite3 library: ${QT_SQLITE3_LIB}")
  SET(QT_SQLITE_FOUND TRUE)
  SET(QT_SQLITE_FALLBACK FALSE)
else(QT_SQLITE3_LIB)
  if(SQLITE_FOUND)
    SET(QSQLITE3_DIR ${CMAKE_BINARY_DIR}/qt-sqlite3-0.2)
    IF(NOT EXISTS ${QSQLITE3_DIR})
      message(STATUS "Qt-Sqlite3 not found in ${QSQLITE3_INSTALL_DIR}
      No problem, extracting and compiling our fallback library.")
      EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E tar xzf
        ${CMAKE_SOURCE_DIR}/23011-qt-sqlite3-0.2.tar.gz
        ${CMAKE_BINARY_DIR})
    ENDIF(NOT EXISTS ${QSQLITE3_DIR})

    GET_FILENAME_COMPONENT(QT_BIN_DIR ${QT_MOC_EXECUTABLE} PATH)

    FILE(GLOB_RECURSE QSQLITE3_SOURCES "${QSQLITE3_DIR}/*.cpp")

    ADD_LIBRARY(qsqlite3 SHARED ${QSQLITE3_SOURCES})
    SET_TARGET_PROPERTIES(qsqlite3 PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY ${QSQLITE3_DIR}/sqldrivers/)
    TARGET_LINK_LIBRARIES(qsqlite3 ${QT_AND_KDECORE_LIBS} ${SQLITE_LIBRARIES})

    INSTALL(TARGETS qsqlite3
      DESTINATION ${QSQLITE3_INSTALL_DIR}
      COMPONENT qsqlite3)
    ADD_CUSTOM_TARGET(install-qsqlite3 ${CMAKE_COMMAND}
      -DCMAKE_INSTALL_COMPONENT=qsqlite3 -P cmake_install.cmake
      DEPENDS qsqlite3)

    SET(QT_SQLITE_FOUND TRUE)
    SET(QT_SQLITE_FALLBACK TRUE)
  else(SQLITE_FOUND)
    message(STATUS "Qt-Sqlite3 not found, but also Sqlite3 is missing. Sqlite3 support disabled.")
    SET(QT_SQLITE_FOUND FALSE)
    SET(QT_SQLITE_FALLBACK FALSE)
  endif(SQLITE_FOUND)
endif(QT_SQLITE3_LIB)
