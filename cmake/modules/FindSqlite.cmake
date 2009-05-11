# Find whether Sqlite3 is installed
#
# Variables:
#  SQLITE_FOUND - system has Sqlite
#  SQLITE_INCLUDE_DIR - the Sqlite include directory
#  SQLITE_LIBRARIES - Link these to use Sqlite
#  SQLITE_DEFINITIONS - Compiler switches required for using Sqlite

if ( SQLITE_INCLUDE_DIR AND SQLITE_LIBRARIES )
   # in cache already
   SET(Sqlite_FIND_QUIETLY TRUE)
endif ( SQLITE_INCLUDE_DIR AND SQLITE_LIBRARIES )

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
if( NOT WIN32 )
  find_package(PkgConfig)

  pkg_check_modules(PC_SQLITE sqlite3)

  set(SQLITE_DEFINITIONS ${PC_SQLITE_CFLAGS_OTHER})
endif( NOT WIN32 )

FIND_PATH(SQLITE_INCLUDE_DIR NAMES sqlite3.h
  PATHS
  ${PC_SQLITE_INCLUDEDIR}
  ${PC_SQLITE_INCLUDE_DIRS}
)

FIND_LIBRARY(SQLITE_LIBRARIES NAMES sqlite3
  PATHS
  ${PC_SQLITE_LIBDIR}
  ${PC_SQLITE_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Sqlite DEFAULT_MSG SQLITE_INCLUDE_DIR SQLITE_LIBRARIES)

MARK_AS_ADVANCED(SQLITE_INCLUDE_DIR SQLITE_LIBRARIES)

SET(SQLITE_LIB_DIRS ${PC_SQLITE_LIBDIR} ${PC_SQLITE_LIBRARY_DIRS})
