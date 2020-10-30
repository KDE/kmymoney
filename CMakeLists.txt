# The CMake version we require
cmake_minimum_required(VERSION 2.8.9)

# Setting the name of the main project
project(KMyMoney)

# Automoc all sources
set(CMAKE_AUTOMOC TRUE)

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
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

######################### KMyMoney Version ##########################

# The current version of KMyMoney (used for packages and config.h)
set(VERSION_MAJOR "4")
set(VERSION_MINOR "8")
set(VERSION_PATCH "5")

# Determine the GIT reference (if we're based on GIT)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git AND CMAKE_SYSTEM_NAME MATCHES Linux)
  # make sure we don't use translated messages here
  set(LANG $ENV{LANG})
  set(ENV{LANG} "C")
  execute_process(COMMAND git log -1 COMMAND head -n 1 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE BRANCH)
  # extract branch name
  string(REGEX MATCH "commit ([^$]+)" BRANCH "${BRANCH}")
  set(BRANCH ${CMAKE_MATCH_1})
  # and use the first 10 chars of the commit id
  string(SUBSTRING ${BRANCH} 0 10 BRANCH)
  set(VERSION_SUFFIX "-${BRANCH}")
  # switch back to the original language setting
  set(ENV{LANG} ${LANG})
endif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git AND CMAKE_SYSTEM_NAME MATCHES Linux)

set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_SUFFIX})

######################### General Requirements ##########################

# setup default CMAKE_INSTALL_PREFIX before calling FIND_PACKAGE(KDE4)
# see https://bugs.kde.org/show_bug.cgi?id=261132
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  execute_process(COMMAND kde4-config --prefix COMMAND tr -d "\n" OUTPUT_VARIABLE KDE4_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "${KDE4_INSTALL_PREFIX}" CACHE PATH
    "KMyMoney install prefix defaults to the KDE4 install prefix: ${KDE4_INSTALL_PREFIX}" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

find_package(KDE4 4.6.0 REQUIRED)

if(NOT TARGET Qt4::moc)
  add_executable(Qt4::moc IMPORTED)
  set_target_properties(Qt4::moc PROPERTIES
    IMPORTED_LOCATION "${QT_MOC_EXECUTABLE}"
  )
endif(NOT TARGET Qt4::moc)

include( KDE4Defaults )
include( MacroLibrary )
include( KMyMoneyMacros )
include( GenerateExportHeader )
# needed by libkgpgfile
if (WIN32)
  find_package(Boost "1.33.1")
else (WIN32)
  find_package(Boost "1.33.1" COMPONENTS graph)
endif (WIN32)
# needed by libkgpgfile
find_package(Gpgmepp 1.7.0)
set(GPGMEPP_LIBS Gpgmepp)
if (NOT Gpgmepp_FOUND)
  find_package(QGpgme REQUIRED)
  set(GPGMEPP_LIBS ${QGPGME_LIBRARIES})
else (NOT Gpgmepp_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif (NOT Gpgmepp_FOUND)
find_package(KdepimLibs)
find_package(SharedMimeInfo REQUIRED)

# set the configuration switch signaling that KDE PIM libraries were found
if (KdepimLibs_FOUND)
  set(HAVE_KDEPIMLIBS 1)
  if(WIN32)
    set(KDEPIMLIBS_AKONADI_CONTACT_LIBS)
  endif()
endif (KdepimLibs_FOUND)

add_definitions( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${KDEPIM_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
if(CMAKE_BUILD_TYPE_TOLOWER MATCHES debugfull)
  add_definitions (-DQT_STRICT_ITERATORS)
  set(KMM_DEBUG 1)
endif()

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

option(BUILD_APIDOC "Create api documentation" ON)
option(BUILD_QCH "Build with QCH support" ON)

# check for Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND AND BUILD_APIDOC)
  find_program(QHELPGENERATOR_EXECUTABLE qhelpgenerator)
  set(DOXYGEN_CALLER_GRAPH           NO)
  set(DOXYGEN_CALL_GRAPH             NO)
  set(DOXYGEN_DOT_GRAPH_MAX_NODES    1000)
  set(DOXYGEN_ENUM_VALUES_PER_LINE   1)
  set(DOXYGEN_EXCLUDE                ${CMAKE_CURRENT_SOURCE_DIR}/libkdchart/ ${CMAKE_CURRENT_BINARY_DIR})
  set(DOXYGEN_EXPAND_AS_DEFINED      ONLINETASK_META)
  set(DOXYGEN_EXPAND_ONLY_PREDEF     YES)
  set(DOXYGEN_EXTRACT_ALL            YES)
  set(DOXYGEN_EXTRACT_PRIVATE        YES)
  set(DOXYGEN_EXTRACT_STATIC         YES)
  set(DOXYGEN_FILE_PATTERNS          *.h *.hh *.hpp *.H *.cpp)
  set(DOXYGEN_FULL_PATH_NAMES        NO)
  set(DOXYGEN_GENERATE_LATEX         NO)
  set(DOXYGEN_GENERATE_TAGFILE       ${CMAKE_CURRENT_BINARY_DIR}/kmymoney.tag)
  set(DOXYGEN_HAVE_DOT               YES)
  set(DOXYGEN_HTML_TIMESTAMP         YES)
  set(DOXYGEN_INCLUDE_PATH           ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR})
  set(DOXYGEN_MACRO_EXPANSION        YES)
  set(DOXYGEN_OUTPUT_DIRECTORY       ${CMAKE_CURRENT_BINARY_DIR}/apidocs)
  set(DOXYGEN_PAPER_TYPE             a4wide)
  set(DOXYGEN_PDF_HYPERLINKS         NO)
  set(DOXYGEN_PROJECT_NAME           KMyMoney)
  set(DOXYGEN_PROJECT_NUMBER         ${VERSION})
  set(DOXYGEN_QUIET                  YES)
  set(DOXYGEN_RECURSIVE              YES)
  set(DOXYGEN_REFERENCED_BY_RELATION YES)
  set(DOXYGEN_REFERENCES_RELATION    YES)
  set(DOXYGEN_SOURCE_BROWSER         YES)
  set(DOXYGEN_UML_LOOK               YES)
  set(DOXYGEN_TEMPLATE_RELATIONS     YES)
  if(QHELPGENERATOR_EXECUTABLE AND BUILD_QCH)
    set(DOXYGEN_GENERATE_QHP           YES)
    set(DOXYGEN_QCH_FILE               ${CMAKE_CURRENT_BINARY_DIR}/kmymoney.qch)
    set(DOXYGEN_QHG_LOCATION           ${QHELPGENERATOR_EXECUTABLE})
    set(DOXYGEN_QHP_NAMESPACE          org.kde.kmymoney)
    install(FILES ${DOXYGEN_QCH_FILE} DESTINATION ${QT_DOC_DIR}/qch COMPONENT Devel)
  endif()

  doxygen_add_docs(apidoc
      ${CMAKE_SOURCE_DIR}
      COMMENT "generating api doc"
  )

  install(DIRECTORY ${DOXYGEN_OUTPUT_DIRECTORY} DESTINATION ${HTML_INSTALL_DIR} COMPONENT Devel)
endif(DOXYGEN_FOUND)

find_package(LibAlkimia REQUIRED)
if(NOT GMP_LIBRARIES)
    set(GMP_LIBRARIES gmp)
endif()
if("${LibAlkimia_VERSION_MAJOR}" EQUAL 7 OR "${LibAlkimia_VERSION_MAJOR}" GREATER 7)
  set(LIBALKIMIA_LIBRARIES Alkimia::alkimia)
else()
  #TODO: remove when LibAlkimia minimum version will be raised
  if (NOT "${LIBALKIMIA_LIBRARIES}" AND "${LIBALKIMIA_LIBRARY}")
    # LibAlkimia 4.3.2 does not properly define LIBALKIMIA_LIBRARIES
    find_package(GMP REQUIRED)
    set(LIBALKIMIA_LIBRARIES ${GMP_LIBRARIES} ${LIBALKIMIA_LIBRARY} )
  endif()
endif()
# make sure LIBALKIMIA_LIBRARIES is set
if("${LibAlkimia_FOUND}" AND NOT "${LIBALKIMIA_LIBRARIES}")
  set(LIBALKIMIA_LIBRARIES Alkimia::alkimia)
endif()
# alkimia 4.3
if("${LIBALKIMIA_FOUND}" AND NOT "${LIBALKIMIA_LIBRARIES}")
    set(LIBALKIMIA_LIBRARIES alkimia)
endif()
# make sure we have the matching version of LibAlkimia (not too new)
if(${LibAlkimia_FOUND})
    if (DEFINED ${LibAlkimia_VERSION})
        if (NOT "${LibAlkimia_VERSION}" VERSION_LESS "6.0.0")
            message(FATAL_ERROR "This version of KMyMoney requires LibAlkimia < 6.0.0 and does not work with the installed version of LibAlkimia")
        endif()
    endif()
endif()

# check for PkgConfig
find_package(PkgConfig)

# check some functions
include (CheckFunctionExists)

# check for function in header
include(CheckSymbolExists)

# check for struct members
include(CheckStructHasMember)

set(CMAKE_REQUIRED_DEFINITIONS -D_XOPEN_SOURCE=500 -D_BSD_SOURCE)

# check some include files exists
include (CheckIncludeFile)
check_include_file("stdint.h" HAVE_STDINT_H)
check_include_file("sys/types.h" HAVE_SYS_TYPES_H)
check_include_file("unistd.h" HAVE_UNISTD_H)

#check sizes. Will be written to config.h
include(CheckTypeSize)
check_type_size(long SIZEOF_LONG)

mark_as_advanced(
  KDE4_KCFGC_EXECUTABLE KDE4_KDECORE_LIBRARY KDECONFIG_EXECUTABLE)

######################### Special Requirements ##########################

# This is needed for QtSqlite and QtDesigner
# (they'll install files to ${QT_INSTALL_DIR}/plugins/)
get_filename_component(QT_BIN_DIR ${QT_MOC_EXECUTABLE} PATH)
get_filename_component(QT_DIR     ${QT_BIN_DIR}        PATH)
set(QT_INSTALL_DIR ${QT_DIR} CACHE PATH
  "Qt install prefix defaults to the Qt prefix: ${QT_DIR}")


# check for optional OFX support
set(LIBOFX_DEFAULT "AUTO")
if(DEFINED ENABLE_LIBOFX)
  set(LIBOFX_DEFAULT ${ENABLE_LIBOFX})
endif(DEFINED ENABLE_LIBOFX)
option(ENABLE_LIBOFX "Enable OFX plugin" ON)
if(ENABLE_LIBOFX)
  find_package(LibOfx)
  if(NOT LIBOFX_FOUND)
    if(NOT LIBOFX_DEFAULT STREQUAL "AUTO")
      message(FATAL_ERROR "LibOFX not found")
    endif(NOT LIBOFX_DEFAULT STREQUAL "AUTO")
    set(ENABLE_LIBOFX OFF CACHE BOOL "Enable OFX plugin" FORCE)
  else(NOT LIBOFX_FOUND)
    check_struct_has_member("struct OfxFiLogin" "clientuid" "libofx/libofx.h" LIBOFX_HAVE_CLIENTUID)
  endif(NOT LIBOFX_FOUND)
endif(ENABLE_LIBOFX)

# check for optional KBanking support
set(KBANKING_FOUND "AUTO")
mark_as_advanced(KBANKING_FOUND)
if(DEFINED ENABLE_KBANKING)
  set(KBANKING_FOUND OFF)
endif(DEFINED ENABLE_KBANKING)
option(ENABLE_KBANKING "Enable KBanking plugin" ON)
if(ENABLE_KBANKING)
  find_package(GWENHYWFAR CONFIG "4.20")
  find_package(gwengui-qt4 CONFIG)
  find_package(gwengui-cpp CONFIG)
  message(STATUS "Gwenhywfar ${GWENHYWFAR_VERSION} found at ${GWENHYWFAR_INCLUDE_DIRS} (${GWENHYWFAR_LIBRARIES})")
  find_package(AQBANKING CONFIG "5.8")
  message(STATUS "AqBanking ${AQBANKING_VERSION} found at ${AQBANKING_INCLUDE_DIRS} (${AQBANKING_LIBRARIES})")
  if (AQBANKING_FOUND AND GWENHYWFAR_FOUND AND gwengui-qt4_FOUND AND gwengui-cpp_FOUND)
    set(KBANKING_FOUND ON)
    add_definitions(-DENABLE_KBANKING)
  else ()
    if(NOT KBANKING_FOUND STREQUAL "AUTO")
      message(FATAL_ERROR "KBanking requirements not met")
    endif(NOT KBANKING_FOUND STREQUAL "AUTO")
    set(KBANKING_FOUND OFF)
    set(ENABLE_KBANKING OFF CACHE BOOL "Enable KBanking plugin" FORCE)
  endif ()
endif(ENABLE_KBANKING)

# check for optional Weboob support
set(WEBOOB_FOUND "AUTO")
mark_as_advanced(WEBOOB_FOUND)
if(DEFINED ENABLE_WEBOOB)
  set(WEBOOB_FOUND OFF)
endif(DEFINED ENABLE_WEBOOB)
option(ENABLE_WEBOOB "Enable weboob plugin" ON)
if(ENABLE_WEBOOB)
  set(WEBOOB_FOUND ON)
endif(ENABLE_WEBOOB)

# check for optional ical support
set(LIBICAL_DEFAULT "AUTO")
if(DEFINED ENABLE_LIBICAL)
  set(LIBICAL_DEFAULT ${ENABLE_LIBICAL})
endif(DEFINED ENABLE_LIBICAL)
option(ENABLE_LIBICAL "Enable Calendar plugin" ON)
if(ENABLE_LIBICAL)
  find_package(Libical)
  if(NOT LIBICAL_FOUND)
    if(NOT LIBICAL_DEFAULT STREQUAL "AUTO")
      message(FATAL_ERROR "LIBICAL not found")
    endif(NOT LIBICAL_DEFAULT STREQUAL "AUTO")
    set(ENABLE_LIBICAL OFF CACHE BOOL "Enable Calendar plugin" FORCE)
  endif(NOT LIBICAL_FOUND)
endif(ENABLE_LIBICAL)

# check for optional KDE Activities support
find_package(KActivities)

# do we build the tests?
if( KDE4_BUILD_TESTS )
  # INCLUDE(CTest) # (makes sense only with a ctest online dashboard)
  enable_testing()
endif( KDE4_BUILD_TESTS )

######################### Settings ##########################

# If no build type is set, use "Release with Debug Info"
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
  "Choose the type of build.
Possible values are: 'Release' 'RelWithDebInfo' 'Debug' 'Debugfull' 'Profile'
The default value is: 'RelWithDebInfo'" FORCE)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

# tell the KMyMoney sources that a config.h is present:
add_definitions( -DHAVE_CONFIG_H )

if(LIBOFX_HAVE_CLIENTUID)
  add_definitions( -DLIBOFX_HAVE_CLIENTUID )
endif(LIBOFX_HAVE_CLIENTUID)

# preprocessor definitions in case this is a debug build
string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_TOLOWER)
if(${CMAKE_BUILD_TYPE_TOLOWER} MATCHES "^debug(full)?$|^profile$")
  add_definitions( -DKMM_DEBUG )
endif()

set(CMAKE_CXX_STANDARD 11)
# tells gcc to enable exception handling
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

if(CMAKE_SYSTEM_NAME MATCHES Linux)
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--as-needed")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
endif(CMAKE_SYSTEM_NAME MATCHES Linux)

if(NOT WIN32)
  # use position independent code
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif(NOT WIN32)
if(CMAKE_COMPILER_IS_GNUCXX)
  # be more pedantic about common symbols
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-common")

  # Debug, Debugfull, Profile
  set(CMAKE_CXX_FLAGS_DEBUG
    "-g -O2 -fno-reorder-blocks -fno-schedule-insns -fno-inline")
  set(CMAKE_CXX_FLAGS_DEBUGFULL
    "-g3 -fno-inline")
  set(CMAKE_CXX_FLAGS_PROFILE
    "-g3 -fno-inline -ftest-coverage -fprofile-arcs")

  # be pedantic about undefined symbols when linking shared libraries
  if(CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME MATCHES kFreeBSD OR CMAKE_SYSTEM_NAME STREQUAL GNU)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wlogical-op")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined -Wl,--as-needed")
  endif(CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME MATCHES kFreeBSD OR CMAKE_SYSTEM_NAME STREQUAL GNU)
endif(CMAKE_COMPILER_IS_GNUCXX)

option(USE_QT_DESIGNER
  "Install KMyMoney specific widget library for Qt-Designer (default=OFF)" OFF)

option(USE_DEVELOPER_DOC
  "Install KMyMoney specific developer documentation (default=OFF)" OFF)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/doc")
  option(USE_HTML_HANDBOOK
    "Generate KMyMoney manual in HTML format (default=OFF)" OFF)
endif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/doc")

# remark: these directories are used to find libs and .h files.
# they are set globally after this point. no need to add everywhere.
link_directories( ${KDE4_LIB_DIR} )
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE4_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
  ${PKGS_INCLUDE_DIRS})


# Generation of the HTML version of the manual
if(USE_HTML_HANDBOOK)
  # to be done. See KDE4Macros.cmake for custom target "htmlhandbook"
  # and kmymoney2/doc/en/Makefile.am on how one could do that
endif(USE_HTML_HANDBOOK)

######################### The Actual Targets ##########################
add_subdirectory( libkgpgfile )
add_subdirectory( libkdchart )
add_subdirectory( tools )
add_subdirectory( kmymoney )
add_subdirectory( doc )
if (USE_DEVELOPER_DOC)
  add_subdirectory( developer-doc )
endif (USE_DEVELOPER_DOC)

#CONFIGURE_FILE(
#  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
#  "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake"
#  IMMEDIATE @ONLY)

#ADD_CUSTOM_TARGET(uninstall
#  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")


# by default, the package name is kmymoney
set(CPACK_PACKAGE_NAME "kmymoney")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A Personal Finance Manager")
set(CPACK_PACKAGE_VERSION_MAJOR  ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR  ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH  ${VERSION_PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "/opt/kde4")

# When building with 'make package', use these cpack generators
set(CPACK_GENERATOR "TGZ" "TBZ2")
# When building with 'make package_source', use these cpack generators
set(CPACK_SOURCE_GENERATOR "TGZ" "TBZ2")

# don't include files matching these into the source packages
# (Note: all other files in the source directory will be included)
set(CPACK_SOURCE_IGNORE_FILES
  "${CMAKE_CURRENT_BINARY_DIR}" # the build directory. MUST build out-of-source!
  "/CVS/"      # cvs directories
  "/\\\\."     # all files starting with '.' (this includes .cvsignore files)
  "~$"         # all files ending in '~'
  "\\\\.m4$"   # since this is a cmake dist, we remove automake files
  "\\\\.m4\\\\.in$" "\\\\.am$" "Makefile\\\\.dist" "configure\\\\.in"
  )
# This will create the cpack configuration files.
include(CPack)

##### RPM Setup (.rpm and .srpm) #####
# We use cmake/modules/UseRPMTools.cmake for RPM creation
include(UseRPMTools)
if(RPMTools_FOUND)
  set(RPM_VERSION_SUFFIX  ${VERSION_SUFFIX})
  if(${RPM_VERSION_SUFFIX} MATCHES "-.*")
    # remove the leading dash from the version suffix
    string(LENGTH ${RPM_VERSION_SUFFIX} LEN)
    math(EXPR LEN ${LEN}-1)
    string(SUBSTRING ${RPM_VERSION_SUFFIX} 1 ${LEN} RPM_VERSION_SUFFIX)
  endif(${RPM_VERSION_SUFFIX} MATCHES "-.*")

  # kmymoney conflicts with kmymoney-unstable
  set(RPM_CONFLICTING kmymoney-unstable)
  rpmtools_add_rpm_targets(kmymoney          kmymoney.spec.in)

  # kmymoney-unstable conflicts with kmymoney
  set(RPM_CONFLICTING kmymoney)
  rpmtools_add_rpm_targets(kmymoney-unstable kmymoney.spec.in)
endif(RPMTools_FOUND)

######################### Output Results #############################

# create the config.h file out of the config.h.cmake
configure_file  (config-kmymoney.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kmymoney.h )
configure_file  (config-kmymoney-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kmymoney-version.h )

# this macro maps the boolean variable ${_varname} to "yes"/"no"
# and writes the output to the variable nice_${_varname}
macro(nice_yesno _varname)
  if(${_varname})
    set("nice_${_varname}" "yes")
  else(${_varname})
    set("nice_${_varname}" "no")
  endif(${_varname})
endmacro(nice_yesno _varname)

nice_yesno("KdepimLibs_FOUND")
nice_yesno("LIBOFX_FOUND")
nice_yesno("LIBOFX_HAVE_CLIENTUID")
nice_yesno("KBANKING_FOUND")
nice_yesno("WEBOOB_FOUND")
nice_yesno("LIBICAL_FOUND")
nice_yesno("ENABLE_SQLCIPHER")
nice_yesno("USE_QT_DESIGNER")
nice_yesno("USE_DEVELOPER_DOC")
nice_yesno("KDE4_BUILD_TESTS")
nice_yesno("DOXYGEN_FOUND")
nice_yesno("USE_HTML_HANDBOOK")
message("
-------- KMyMoney ${CPACK_PACKAGE_VERSION}${VERSION_SUFFIX} --------

Configure results (user options):
--------------------------------------------
KDE PIM holidays:                        ${nice_KdepimLibs_FOUND}

OFX plugin:                              ${nice_LIBOFX_FOUND}
OFX clientuid support                    ${nice_LIBOFX_HAVE_CLIENTUID}

KBanking plugin:                         ${nice_KBANKING_FOUND}

weboob plugin:                           ${nice_WEBOOB_FOUND}

iCalendar export plugin:                 ${nice_LIBICAL_FOUND}

SQLCipher plugin:                        ${nice_ENABLE_SQLCIPHER}
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
