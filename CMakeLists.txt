# Setting the name of the main project
PROJECT(KMyMoney)

# The CMake version we require
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# cmake 2.6 proposed to add this policy
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# Our modules reside in the subdirectory cmake/modules/
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules)

######################### KMyMoney Version ##########################

# The current version of KMyMoney (used for packages and config.h)
SET(VERSION_MAJOR "3")
SET(VERSION_MINOR "95")
SET(VERSION_PATCH "0")

# Get the date of the last change from the ChangeLog
SET(date_format "([0-9][0-9][0-9][0-9])-([0-9][0-9])-([0-9][0-9])")
FILE(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/ChangeLog LAST_CHANGE
  REGEX ${date_format} LIMIT_COUNT 1 LENGTH_MINIMUM 10 LENGTH_MAXIMUM 10)
STRING(REGEX MATCH ${date_format} _ign "${LAST_CHANGE}")
SET(VERSION_SUFFIX "svn${CMAKE_MATCH_1}${CMAKE_MATCH_2}${CMAKE_MATCH_3}")

######################### General Requirements ##########################
FIND_PACKAGE(KDE4 REQUIRED)


include( KDE4Defaults )
include( MacroLibrary )
find_package(KdepimLibs REQUIRED)

add_definitions( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

include_directories( ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} 
	${CMAKE_CURRENT_BINARY_DIR}/kmymoney2/dialogs/
	${CMAKE_CURRENT_SOURCE_DIR}/kmymoney2/widgets/
	${CMAKE_CURRENT_BINARY_DIR}/kmymoney2/widgets/
	${CMAKE_CURRENT_SOURCE_DIR}/kmymoney2/mymoney/
	${CMAKE_CURRENT_SOURCE_DIR}/kmymoney2/mymoney/storage/
  ${CMAKE_CURRENT_SOURCE_DIR}/libkgpgfile/
	${CMAKE_CURRENT_SOURCE_DIR}/kmymoney2/plugins/
	${CMAKE_CURRENT_SOURCE_DIR}/kmymoney2/views/
	${CMAKE_CURRENT_SOURCE_DIR}/kmymoney2/dialogs/
	${CMAKE_CURRENT_SOURCE_DIR}/kmymoney2/converter/
	${CMAKE_CURRENT_BINARY_DIR}/kmymoney2/dialogs/settings/
	${CMAKE_CURRENT_BINARY_DIR}/kmymoney2/mymoney/storage/
	${CMAKE_CURRENT_SOURCE_DIR}/kmymoney2/reports/
	${CMAKE_CURRENT_SOURCE_DIR}/kmymoney2/wizards/wizardpages/)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )
string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)

if(CMAKE_BUILD_TYPE_TOLOWER MATCHES debugfull)
  set(KMM_DEBUG 1)
endif()


# check for JPEG
FIND_PACKAGE(JPEG)
IF (JPEG_FOUND)
  SET(HAVE_LIBJPEG 1)
ENDIF (JPEG_FOUND)

# check for PNGL
FIND_PACKAGE(PNG)
IF (PNG_FOUND)
  SET(HAVE_LIBPNG 1)
ENDIF (PNG_FOUND)

# check for THREAD model
Find_PACKAGE(Threads)
IF (CMAKE_USE_PTHREADS_INIT)
  SET(HAVE_LIBPTHREAD 1)
ENDIF (CMAKE_USE_PTHREADS_INIT)

# check for ZLIB
FIND_PACKAGE(ZLIB)
IF (ZLIB_FOUND)
  SET(HAVE_LIBZ 1)
ENDIF (ZLIB_FOUND)

# check for PkgConfig
FIND_PACKAGE(PkgConfig)
# check for some modules we require
pkg_check_modules (PKGS libxml-2.0 libxml++-2.6 glib-2.0 glibmm-2.4)
# this sets variables like ${PKGS_INCLUDE_DIRS} ${PKGS_LIBRARY_DIRS}
# (cf. /usr/share/cmake/Modules/FindPkgConfig.cmake for more info)

# check some functions
INCLUDE (CheckFunctionExists) 
CHECK_FUNCTION_EXISTS(atoll HAVE_ATOLL)
CHECK_FUNCTION_EXISTS(vsnprintf HAVE_VSNPRINTF)
CHECK_FUNCTION_EXISTS(strtoll HAVE_STRTOLL)

SET(CMAKE_REQUIRED_FLAGS "-std=c99")
SET(CMAKE_REQUIRED_LIBRARIES m)
CHECK_FUNCTION_EXISTS(round HAVE_ROUND)

# check for function in header
INCLUDE(CheckSymbolExists)
SET(CMAKE_REQUIRED_DEFINITIONS -D_XOPEN_SOURCE=500 -D_BSD_SOURCE)
CHECK_SYMBOL_EXISTS(res_init "sys/types.h;netinet/in.h;arpa/nameser.h;resolv.h" HAVE_RES_INIT)

# check for res_init with prototype
INCLUDE(CheckPrototypeExists)
CHECK_PROTOTYPE_EXISTS(res_init "sys/types.h;netinet/in.h;arpa/nameser.h;resolv.h" HAVE_RES_INIT_PROTO)

# special handling of crypt() check
# ----------------------- crypt() ---------------------------------
SET(CMAKE_REQUIRED_LIBRARIES crypt) 
CHECK_FUNCTION_EXISTS(crypt HAVE_CRYPT)
# -----------------------------------------------------------------

# check some include files exists
INCLUDE (CheckIncludeFile)
CHECK_INCLUDE_FILE("stdint.h" HAVE_STDINT_H)
CHECK_INCLUDE_FILE("dlfcn.h" HAVE_DLFCN_H)
CHECK_INCLUDE_FILE("inttypes.h" HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE("memory.h" HAVE_MEMORY_H)
CHECK_INCLUDE_FILE("stdlib.h" HAVE_STDLIB_H)
CHECK_INCLUDE_FILE("strings.h" HAVE_STRINGS_H)
CHECK_INCLUDE_FILE("string.h" HAVE_STRING_H)
CHECK_INCLUDE_FILE("sys/bitypes.h" HAVE_SYS_BITYPES_H)
CHECK_INCLUDE_FILE("sys/stat.h" HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE("sys/types.h" HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE("unistd.h" HAVE_UNISTD_H)

#check sizes. Will be written to config.h
INCLUDE(CheckTypeSize)
CHECK_TYPE_SIZE(char* SIZEOF_CHAR_P)
CHECK_TYPE_SIZE(int SIZEOF_INT)
CHECK_TYPE_SIZE(long SIZEOF_LONG)
CHECK_TYPE_SIZE(short SIZEOF_SHORT)
CHECK_TYPE_SIZE(size_t SIZEOF_SIZE_T)
CHECK_TYPE_SIZE("unsigned long" SIZEOF_UNSIGNED_LONG)

MARK_AS_ADVANCED(
  KDE4_KCFGC_EXECUTABLE KDE4_KDECORE_LIBRARY KDECONFIG_EXECUTABLE)

######################### Special Requirements ##########################

CHECK_INCLUDE_FILE("libofx/libofx.h" HAVE_LIBOFX_H)
IF(HAVE_LIBOFX_H)
  OPTION(USE_OFX_DIRECTCONNECT "Enable to compile with ofximport support" ON)
  # the USE_OFX_DIRECTCONNECT preprocessor definition will be written
  # to config.h as specified in config.h.cmake
ENDIF(HAVE_LIBOFX_H)

# This is needed for QtSqlite and QtDesigner
# (they'll install files to ${QT_INSTALL_DIR}/plugins/)
GET_FILENAME_COMPONENT(QT_BIN_DIR ${QT_MOC_EXECUTABLE} PATH)
GET_FILENAME_COMPONENT(QT_DIR     ${QT_BIN_DIR}        PATH)
SET(QT_INSTALL_DIR ${QT_DIR} CACHE PATH
  "Qt install prefix defaults to the Qt prefix: ${QT_DIR}")

FIND_PACKAGE(Libical)

# check for CppUnit to build the tests
FIND_PACKAGE(CppUnit)
IF( CPPUNIT_FOUND )
  SET(HAVE_LIBCPPUNIT 1)
  INCLUDE_DIRECTORIES( ${CPPUNIT_INCLUDE_DIR} )
ENDIF( CPPUNIT_FOUND )

# check wether the user requests to build the test although it's not possible
IF( KDE4_BUILD_TESTS AND NOT CPPUNIT_FOUND )
  message( SEND_ERROR
    "CppUnit has not been found. Tests have been deactivated." )
  SET( KDE4_BUILD_TESTS OFF FORCE )
ENDIF( KDE4_BUILD_TESTS AND NOT CPPUNIT_FOUND )

# do we build the tests?
IF( KDE4_BUILD_TESTS )
  # INCLUDE(CTest) # (makes sense only with a ctest online dashboard)
  ENABLE_TESTING()
ENDIF( KDE4_BUILD_TESTS )

######################### Settings ##########################

# If no build type is set, use "Release with Debug Info"
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE RelWithDebInfo)
ENDIF(NOT CMAKE_BUILD_TYPE)

SET(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
  "Choose the type of build.
Possible values are: 'Release' 'RelWithDebInfo' 'Debug' 'Debugfull' 'Profile'
The default value is: 'RelWithDebInfo'" FORCE)

ADD_DEFINITIONS(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

# tell the KMyMoney sources that a config.h is present:
ADD_DEFINITIONS( -DHAVE_CONFIG_H )

# preprocessor definitions in case this is a debug build
STRING(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_TOLOWER)
IF(${CMAKE_BUILD_TYPE_TOLOWER} MATCHES "^debug(full)?$|^profile$")
  ADD_DEFINITIONS( -DKMM_DEBUG )
ENDIF()

# tells gcc to enable exception handling
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
# use position independent code
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
# be more pedantic about common symbols
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-common")

# Debug, Debugfull, Profile
SET(CMAKE_CXX_FLAGS_DEBUG
  "-g -O2 -fno-reorder-blocks -fno-schedule-insns -fno-inline")
SET(CMAKE_CXX_FLAGS_DEBUGFULL
  "-g3 -fno-inline")
SET(CMAKE_CXX_FLAGS_PROFILE
  "-g3 -fno-inline -ftest-coverage -fprofile-arcs")

# be pedantic about undefined symbols when linking shared libraries
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")

OPTION(USE_LEAK_CHECK "Enable to compile with memory leak check support (deprecated)" OFF)
if(USE_LEAK_CHECK)
  message("Warning: Leak check support is activated but deprecated. Better use valgrind.")
  ADD_DEFINITIONS( -D_CHECK_MEMORY )
endif(USE_LEAK_CHECK)

OPTION(USE_QT_DESIGNER
  "Install KMyMoney specific widget library for Qt-Designer (default=OFF)" OFF)

if(USE_QT_DESIGNER)
   set(KMM_DESIGNER 1)
endif(USE_QT_DESIGNER)

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX "${KDE4PREFIX}" CACHE PATH
    "KMyMoney2 install prefix defaults
    to the KDE4 install prefix: ${KDE4PREFIX}" FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# the RPATH to be used when installing
# (cf. http://www.vtk.org/Wiki/CMake_RPATH_handling)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# remark: these directories are used to find libs and .h files.
# they are set globally after this point. no need to add everywhere.
LINK_DIRECTORIES( ${KDE4_LIB_DIR} )
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE4_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
  ${PKGS_INCLUDE_DIRS})

######################### The Actual Targets ##########################

ADD_SUBDIRECTORY( libkgpgfile )
ADD_SUBDIRECTORY( libkdchart )
ADD_SUBDIRECTORY( kmymoney2 )
#ADD_SUBDIRECTORY( doc )
#ADD_SUBDIRECTORY( developer-doc )

#CONFIGURE_FILE(
#  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
#  "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake"
#  IMMEDIATE @ONLY)

#ADD_CUSTOM_TARGET(uninstall
#  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")


# by default, the package name is kmymoney
SET(CPACK_PACKAGE_NAME "kmymoney")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A Personal Finance Manager")
SET(CPACK_PACKAGE_VERSION_MAJOR  ${VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR  ${VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH  ${VERSION_PATCH})
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "/opt/kde4")

# When building with 'make package', use these cpack generators
SET(CPACK_GENERATOR "TGZ" "TBZ2")
# When building with 'make package_source', use these cpack generators
SET(CPACK_SOURCE_GENERATOR "TGZ" "TBZ2")

# don't include files matching these into the source packages
# (Note: all other files in the source directory will be included)
SET(CPACK_SOURCE_IGNORE_FILES
  "${CMAKE_CURRENT_BINARY_DIR}" # the build directory. MUST build out-of-source!
  "/CVS/"      # cvs directories
  "/\\\\."     # all files starting with '.' (this includes .cvsignore files)
  "~$"         # all files ending in '~'
  "\\\\.m4$"   # since this is a cmake dist, we remove automake files
  "\\\\.m4\\\\.in$" "\\\\.am$" "Makefile\\\\.dist" "configure\\\\.in"
  )
# This will create the cpack configuration files.
INCLUDE(CPack)

##### RPM Setup (.rpm and .srpm) #####
# We use cmake/modules/UseRPMTools.cmake for RPM creation
INCLUDE(UseRPMTools)
IF(RPMTools_FOUND)
  SET(RPM_VERSION_SUFFIX  ${VERSION_SUFFIX})

  # kmymoney conflicts with kmymoney-unstable
  SET(RPM_CONFLICTING kmymoney-unstable)
  RPMTools_ADD_RPM_TARGETS(kmymoney          kmymoney.spec.in)

  # kmymoney-unstable conflicts with kmymoney
  SET(RPM_CONFLICTING kmymoney)
  RPMTools_ADD_RPM_TARGETS(kmymoney-unstable kmymoney.spec.in)
ENDIF(RPMTools_FOUND)

######################### Output Results #############################

# create the config.h file out of the config.h.cmake
CONFIGURE_FILE  (config-kmymoney.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kmymoney.h )

# this macro maps the boolean variable ${_varname} to "yes"/"no"
# and writes the output to the variable nice_${_varname}
MACRO(nice_yesno _varname)
  if(${${_varname}})
    SET("nice_${_varname}" "yes")
  else(${${_varname}})
    if(${_varname})
      SET("nice_${_varname}" "yes")
    else(${_varname})
      SET("nice_${_varname}" "no")
    endif(${_varname})
  endif(${${_varname}})
ENDMACRO(nice_yesno _varname)

nice_yesno("HAVE_LIBOFX_H")
nice_yesno("USE_OFX_DIRECTCONNECT")
nice_yesno("LIBICAL_FOUND")
nice_yesno("QT_SQLITE_FOUND")
nice_yesno("USE_LEAK_CHECK")
nice_yesno("USE_QT_DESIGNER")
nice_yesno("HAVE_LIBCPPUNIT")
nice_yesno("KDE4_BUILD_TESTS")
message("
-------- KMyMoney ${CPACK_PACKAGE_VERSION}-${VERSION_SUFFIX} --------

Configure results (user options):
--------------------------------------------
OFX plugin:                              ${nice_HAVE_LIBOFX_H}

OFX direct connect:                      ${nice_USE_OFX_DIRECTCONNECT}

iCalendar export plugin:                 ${nice_LIBICAL_FOUND}

SQLITE3 support:                         ${nice_QT_SQLITE_FOUND}
--------------------------------------------


Configure results (developer options):
--------------------------------------------
Qt-Designer library support:             ${nice_USE_QT_DESIGNER}

PDF document generation:                 ?

CPPUNIT support:                         ${nice_HAVE_LIBCPPUNIT}
        build tests:                     ${nice_KDE4_BUILD_TESTS}

Build type: ${CMAKE_BUILD_TYPE}")

if(USE_LEAK_CHECK)
  message("
Memory leak check support (deprecated):  ${nice_USE_LEAK_CHECK}")
endif(USE_LEAK_CHECK)

message("--------------------------------------------
")

# Display this message the first time only.
if(QT_SQLITE_FALLBACK AND QT_SQLITE_FOUND AND NOT Sqlite_FIND_QUIETLY)
  message("Note: You should run 'make install-qsqlite3' to enable Sqlite3 support.
  ")
endif(QT_SQLITE_FALLBACK AND QT_SQLITE_FOUND AND NOT Sqlite_FIND_QUIETLY)

message("Good - your configure finished.
Now type 'make' to build KMyMoney. For more help, consult README.cmake
")
