# Setting the name of the main project
PROJECT(KMyMoney)

# The CMake version we require
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.4)

# cmake 2.6 proposed to add this policy
if(COMMAND cmake_policy)
  if(POLICY CMP0017)
    cmake_policy(SET CMP0017 NEW)
  endif(POLICY CMP0017)
  cmake_policy(SET CMP0003 NEW)
  # until the createtarball script generates different custom targets
  # for the various translations, we need to keep the following setting
  # http://www.vtkedge.org/Wiki/CMake_Policies#Setting_Policies_Individually
  # has an explanation of the problem
  cmake_policy(SET CMP0002 OLD)
endif(COMMAND cmake_policy)

# Our modules reside in the subdirectory cmake/modules/
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

######################### KMyMoney Version ##########################

# The current version of KMyMoney (used for packages and config.h)
SET(VERSION_MAJOR "4")
SET(VERSION_MINOR "6")
SET(VERSION_PATCH "90")

# Determine the SVN revision number (if we're based on SVN)
IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.svn)
  # make sure we don't use translated messages here
  SET(LANG $ENV{LANG})
  SET(ENV{LANG} "C")
  EXECUTE_PROCESS(COMMAND svn info ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE VERSION_SUFFIX)
  STRING(REGEX MATCH "Revision: ([0-9]+)" VERSION_SUFFIX "${VERSION_SUFFIX}") 
  STRING(REPLACE "Revision: " "" VERSION_SUFFIX "${VERSION_SUFFIX}")
  SET(VERSION_SUFFIX "-svn${VERSION_SUFFIX}")
  # switch back to the original language setting
  SET(ENV{LANG} ${LANG})
ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.svn)

# Determine the GIT reference (if we're based on GIT)
IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git AND CMAKE_SYSTEM_NAME MATCHES Linux)
  # make sure we don't use translated messages here
  SET(LANG $ENV{LANG})
  SET(ENV{LANG} "C")
  EXECUTE_PROCESS(COMMAND git log -1 COMMAND head -n 1 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE BRANCH)
  # extract branch name
  STRING(REGEX MATCH "commit ([^$]+)" BRANCH "${BRANCH}")
  SET(BRANCH ${CMAKE_MATCH_1})
  # and use the first 10 chars of the commit id
  STRING(SUBSTRING ${BRANCH} 0 10 BRANCH)
  SET(VERSION_SUFFIX "-${BRANCH}")
  # switch back to the original language setting
  SET(ENV{LANG} ${LANG})
ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git AND CMAKE_SYSTEM_NAME MATCHES Linux)

######################### General Requirements ##########################

# setup default CMAKE_INSTALL_PREFIX before calling FIND_PACKAGE(KDE4)
# see https://bugs.kde.org/show_bug.cgi?id=261132
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  execute_process(COMMAND kde4-config --prefix COMMAND tr -d "\n" OUTPUT_VARIABLE KDE4_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "${KDE4_INSTALL_PREFIX}" CACHE PATH
    "KMyMoney install prefix defaults to the KDE4 install prefix: ${KDE4_INSTALL_PREFIX}" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

FIND_PACKAGE(KDE4 4.6.0 REQUIRED)

include( KDE4Defaults )
include( MacroLibrary )
include( KMyMoneyMacros )
# needed by libkgpgfile
if (WIN32)
  find_package(Boost "1.33.1")
else (WIN32)
  find_package(Boost "1.33.1" COMPONENTS graph)
endif (WIN32)
# needed by libkgpgfile
find_package(QGpgme REQUIRED)
find_package(KdepimLibs REQUIRED)
find_package(SharedMimeInfo REQUIRED)

add_definitions( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${KDEPIM_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
if(CMAKE_BUILD_TYPE_TOLOWER MATCHES debugfull)
  add_definitions (-DQT_STRICT_ITERATORS)
  set(KMM_DEBUG 1)
endif()

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

# check for Doxygen
FIND_PACKAGE(Doxygen)
IF(DOXYGEN_FOUND)
  SET(APIDOC_DIR ${CMAKE_CURRENT_BINARY_DIR}/apidocs)

  MAKE_DIRECTORY(${APIDOC_DIR})

  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/kmymoney.doxygen.in ${CMAKE_CURRENT_BINARY_DIR}/kmymoney.doxygen IMMEDIATE)

  ADD_CUSTOM_TARGET(apidoc ${DOXYGEN} ${CMAKE_CURRENT_BINARY_DIR}/kmymoney.doxygen)
ENDIF(DOXYGEN_FOUND)

FIND_PACKAGE(GMP REQUIRED)
SET(LIBALKIMIA_MIN_VERSION 4.3.2)
FIND_PACKAGE(LibAlkimia REQUIRED)

# check for PkgConfig
FIND_PACKAGE(PkgConfig)
# check for some modules we require
pkg_check_modules (PKGS libxml-2.0 libxml++-2.6 glib-2.0 glibmm-2.4)
# this sets variables like ${PKGS_INCLUDE_DIRS} ${PKGS_LIBRARY_DIRS}
# (cf. /usr/share/cmake/Modules/FindPkgConfig.cmake for more info)

# check some functions
INCLUDE (CheckFunctionExists) 

IF(NOT WIN32)
    SET(CMAKE_REQUIRED_FLAGS "-std=c99")
    SET(CMAKE_REQUIRED_LIBRARIES m)
ENDIF(NOT WIN32)

# check for function in header
INCLUDE(CheckSymbolExists)
SET(CMAKE_REQUIRED_DEFINITIONS -D_XOPEN_SOURCE=500 -D_BSD_SOURCE)

# check some include files exists
INCLUDE (CheckIncludeFile)
CHECK_INCLUDE_FILE("stdint.h" HAVE_STDINT_H)
CHECK_INCLUDE_FILE("sys/types.h" HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE("unistd.h" HAVE_UNISTD_H)

#check sizes. Will be written to config.h
INCLUDE(CheckTypeSize)
CHECK_TYPE_SIZE(long SIZEOF_LONG)

MARK_AS_ADVANCED(
  KDE4_KCFGC_EXECUTABLE KDE4_KDECORE_LIBRARY KDECONFIG_EXECUTABLE)

######################### Special Requirements ##########################

# This is needed for QtSqlite and QtDesigner
# (they'll install files to ${QT_INSTALL_DIR}/plugins/)
GET_FILENAME_COMPONENT(QT_BIN_DIR ${QT_MOC_EXECUTABLE} PATH)
GET_FILENAME_COMPONENT(QT_DIR     ${QT_BIN_DIR}        PATH)
SET(QT_INSTALL_DIR ${QT_DIR} CACHE PATH
  "Qt install prefix defaults to the Qt prefix: ${QT_DIR}")


# check for optional OFX support
SET(LIBOFX_DEFAULT "AUTO")
IF(DEFINED ENABLE_LIBOFX)
  SET(LIBOFX_DEFAULT ${ENABLE_LIBOFX})
ENDIF(DEFINED ENABLE_LIBOFX)
OPTION(ENABLE_LIBOFX "Enable OFX plugin" ON)
IF(ENABLE_LIBOFX)
  FIND_PACKAGE(LibOfx)
  IF(NOT LIBOFX_FOUND)
    IF(NOT LIBOFX_DEFAULT STREQUAL "AUTO")
      MESSAGE(FATAL_ERROR "LibOFX not found")
    ENDIF(NOT LIBOFX_DEFAULT STREQUAL "AUTO")
    SET(ENABLE_LIBOFX OFF CACHE BOOL "Enable OFX plugin" FORCE)
  ENDIF(NOT LIBOFX_FOUND)
ENDIF(ENABLE_LIBOFX)

# check for optional KBanking support
SET(KBANKING_FOUND "AUTO")
MARK_AS_ADVANCED(KBANKING_FOUND)
IF(DEFINED ENABLE_KBANKING)
  SET(KBANKING_FOUND OFF)
ENDIF(DEFINED ENABLE_KBANKING)
OPTION(ENABLE_KBANKING "Enable KBanking plugin" ON)
IF(ENABLE_KBANKING)
  FIND_PACKAGE(Gwenhywfar)
  FIND_PACKAGE(AqBanking)
  IF (AQBANKING_FOUND AND GWENHYWFAR_FOUND)
    SET(KBANKING_FOUND ON)
  ELSE (AQBANKING_FOUND AND GWENHYWFAR_FOUND)
    IF(NOT KBANKING_FOUND STREQUAL "AUTO")
      MESSAGE(FATAL_ERROR "KBanking requirements not met")
    ENDIF(NOT KBANKING_FOUND STREQUAL "AUTO")
    SET(KBANKING_FOUND OFF)
    SET(ENABLE_KBANKING OFF CACHE BOOL "Enable KBanking plugin" FORCE)
  ENDIF (AQBANKING_FOUND AND GWENHYWFAR_FOUND)
ENDIF(ENABLE_KBANKING)

# check for optional ical support
SET(LIBICAL_DEFAULT "AUTO")
IF(DEFINED ENABLE_LIBICAL)
  SET(LIBICAL_DEFAULT ${ENABLE_LIBICAL})
ENDIF(DEFINED ENABLE_LIBICAL)
OPTION(ENABLE_LIBICAL "Enable Calendar plugin" ON)
IF(ENABLE_LIBICAL)
  FIND_PACKAGE(Libical)
  IF(NOT LIBICAL_FOUND)
    IF(NOT LIBICAL_DEFAULT STREQUAL "AUTO")
      MESSAGE(FATAL_ERROR "LIBICAL not found")
    ENDIF(NOT LIBICAL_DEFAULT STREQUAL "AUTO")
    SET(ENABLE_LIBICAL OFF CACHE BOOL "Enable Calendar plugin" FORCE)
  ENDIF(NOT LIBICAL_FOUND)
ENDIF(ENABLE_LIBICAL)

# check for optional KDE Activities support
FIND_PACKAGE(KActivities)

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

IF(CMAKE_SYSTEM_NAME MATCHES Linux)
  SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--as-needed")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
ENDIF(CMAKE_SYSTEM_NAME MATCHES Linux)

IF(NOT WIN32)
  # use position independent code
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
ENDIF(NOT WIN32)
IF(NOT MSVC)
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
  IF(CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME MATCHES kFreeBSD OR CMAKE_SYSTEM_NAME STREQUAL GNU)
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined -Wl,--as-needed")
  ENDIF(CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME MATCHES kFreeBSD OR CMAKE_SYSTEM_NAME STREQUAL GNU)
ENDIF(NOT MSVC)

OPTION(USE_QT_DESIGNER
  "Install KMyMoney specific widget library for Qt-Designer (default=OFF)" OFF)

OPTION(USE_DEVELOPER_DOC
  "Install KMyMoney specific developer documentation (default=OFF)" OFF)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/doc")
  OPTION(USE_HTML_HANDBOOK
    "Generate KMyMoney manual in HTML format (default=OFF)" OFF)
endif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/doc")

# remark: these directories are used to find libs and .h files.
# they are set globally after this point. no need to add everywhere.
LINK_DIRECTORIES( ${KDE4_LIB_DIR} )
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE4_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
  ${PKGS_INCLUDE_DIRS})


# Generation of the HTML version of the manual
IF(USE_HTML_HANDBOOK)
  # to be done. See KDE4Macros.cmake for custom target "htmlhandbook"
  # and kmymoney2/doc/en/Makefile.am on how one could do that
ENDIF(USE_HTML_HANDBOOK)

######################### The Actual Targets ##########################
ADD_SUBDIRECTORY( libkgpgfile )
ADD_SUBDIRECTORY( libkdchart )
ADD_SUBDIRECTORY( kmymoney )
ADD_SUBDIRECTORY( doc )
IF (USE_DEVELOPER_DOC)
  ADD_SUBDIRECTORY( developer-doc )
ENDIF (USE_DEVELOPER_DOC)

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
  IF(${RPM_VERSION_SUFFIX} MATCHES "-.*")
    # remove the leading dash from the version suffix
    STRING(LENGTH ${RPM_VERSION_SUFFIX} LEN)
    MATH(EXPR LEN ${LEN}-1)
    STRING(SUBSTRING ${RPM_VERSION_SUFFIX} 1 ${LEN} RPM_VERSION_SUFFIX) 
  ENDIF(${RPM_VERSION_SUFFIX} MATCHES "-.*")

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
CONFIGURE_FILE  (config-kmymoney-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kmymoney-version.h )

# this macro maps the boolean variable ${_varname} to "yes"/"no"
# and writes the output to the variable nice_${_varname}
MACRO(nice_yesno _varname)
  if(${_varname})
    SET("nice_${_varname}" "yes")
  else(${_varname})
    SET("nice_${_varname}" "no")
  endif(${_varname})
ENDMACRO(nice_yesno _varname)

nice_yesno("LIBOFX_FOUND")
nice_yesno("KBANKING_FOUND")
nice_yesno("LIBICAL_FOUND")
nice_yesno("USE_QT_DESIGNER")
nice_yesno("USE_DEVELOPER_DOC")
nice_yesno("KDE4_BUILD_TESTS")
nice_yesno("DOXYGEN_FOUND")
nice_yesno("USE_HTML_HANDBOOK")
message("
-------- KMyMoney ${CPACK_PACKAGE_VERSION}${VERSION_SUFFIX} --------

Configure results (user options):
--------------------------------------------
OFX plugin:                              ${nice_LIBOFX_FOUND}

KBanking plugin:                         ${nice_KBANKING_FOUND}

iCalendar export plugin:                 ${nice_LIBICAL_FOUND}
--------------------------------------------


Configure results (developer options):
--------------------------------------------
Qt-Designer library support:             ${nice_USE_QT_DESIGNER}

Generate developer documentation:        ${nice_USE_DEVELOPER_DOC}

Build unit tests:                        ${nice_KDE4_BUILD_TESTS}

Generate API documentation with Doxygen: ${nice_DOXYGEN_FOUND}")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/doc")
  message("
Generate HTML documentation:             ${nice_USE_HTML_HANDBOOK}")

endif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/doc")

message("
Build type: ${CMAKE_BUILD_TYPE}")

message("--------------------------------------------
")

message("Good - your configure finished.
Now type 'make' to build KMyMoney. For more help, consult README.cmake
")
