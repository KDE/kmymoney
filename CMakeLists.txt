# The CMake version we require
cmake_minimum_required(VERSION 3.1)

# Setting the name of the main project
project(KMyMoney VERSION "4.100.0")

# Determine the GIT reference (if we're based on GIT)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
  execute_process(COMMAND git rev-parse --short HEAD WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_VARIABLE VERSION_SUFFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(VERSION_SUFFIX "-${VERSION_SUFFIX}")
  # Add variables which are similar to the build in names of cmake
  set(PROJECT_VERSION_SUFFIX "${VERSION_SUFFIX}")
  set(${PROJECT_NAME}_VERSION_SUFFIX "${VERSION_SUFFIX}")
endif()

# Automoc all sources
set(CMAKE_AUTOMOC TRUE)
list(APPEND CMAKE_AUTOMOC_MACRO_NAMES "K_PLUGIN_FACTORY" "K_PLUGIN_FACTORY_WITH_JSON")

if (POLICY CMP0063)
  cmake_policy(SET CMP0063 NEW) # Policy introduced in CMake version 3.3
endif()


######################### General Requirements ##########################

find_package(ECM 0.0.11 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(FeatureSummary)

include(GenerateExportHeader)
include(KMyMoneyMacros)

set(GPG_ENCRYPTION "no")
set (OPT_KF5_COMPONENTS DocTools Holidays Contacts Akonadi IdentityManagement Activities Kross)
find_package(Gpgmepp)
if (Gpgmepp_FOUND)
  set(GPG_ENCRYPTION "yes")
else()
  set(OPT_KF5_COMPONENTS ${OPT_KF5_COMPONENTS} Gpgmepp)
endif()

find_package(Qt5 5.6 REQUIRED COMPONENTS Core DBus Widgets Svg Sql Xml Test PrintSupport)

find_package(KF5 5.2 REQUIRED
  COMPONENTS Archive CoreAddons Config ConfigWidgets I18n Completion KCMUtils ItemModels ItemViews Service Wallet IconThemes XmlGui TextWidgets Notifications KIO
  OPTIONAL_COMPONENTS ${OPT_KF5_COMPONENTS}
)
find_package(LibAlkimia5 7.0.0 REQUIRED)
find_package(KChart 2.6.0 REQUIRED)

if(KF5Gpgmepp_FOUND)
  set(GPG_ENCRYPTION "yes")
  add_definitions(-DGpgmepp_FOUND)
endif()

add_definitions(-DQT_USE_QSTRINGBUILDER -DQT_NO_CAST_TO_ASCII -DQT_NO_URL_CAST_FROM_STRING)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# use DBus only on Linux
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(KMM_DBUS 1)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

# check for Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
  set(APIDOC_DIR ${CMAKE_CURRENT_BINARY_DIR}/apidocs)

  make_directory("${APIDOC_DIR}")
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmymoney.doxygen.in ${CMAKE_CURRENT_BINARY_DIR}/kmymoney.doxygen IMMEDIATE)

  add_custom_target(apidoc "${DOXYGEN}" "${CMAKE_CURRENT_BINARY_DIR}/kmymoney.doxygen")
endif(DOXYGEN_FOUND)

# check some include files exists
set(CMAKE_REQUIRED_DEFINITIONS -D_XOPEN_SOURCE=500 -D_BSD_SOURCE)
include (CheckIncludeFile)
check_include_file("sys/types.h" HAVE_SYS_TYPES_H)
check_include_file("unistd.h" HAVE_UNISTD_H)
check_include_file("pwd.h" HAVE_PWD_H)
check_include_file("windows.h" HAVE_WINDOWS_H)
check_include_file("lmcons.h" HAVE_LMCONS_H)
check_include_file("process.h" HAVE_PROCESS_H)

# include check for members in structs
include (CheckStructHasMember)

######################### Special Requirements ##########################

# This is needed for QtSqlite and QtDesigner
# (they'll install files to ${QT_INSTALL_DIR}/plugins/)
get_filename_component(QT_BIN_DIR "${QT_MOC_EXECUTABLE}" PATH)
get_filename_component(QT_DIR     ${QT_BIN_DIR}        PATH)
set(QT_INSTALL_DIR ${QT_DIR} CACHE PATH
  "Qt install prefix defaults to the Qt prefix: ${QT_DIR}")

if(KF5IdentityManagement_FOUND AND KF5Akonadi_FOUND AND KF5Contacts_FOUND)
  set(KMM_ADDRESSBOOK_FOUND true)
endif()

# check for optional QWebEngine
option(ENABLE_WEBENGINE "Enable QWebEngine" OFF)
if(ENABLE_WEBENGINE)
  find_package(Qt5WebEngineWidgets 5.8 REQUIRED)
  if(Qt5WebEngineWidgets_VERSION VERSION_GREATER 5.8.99 AND Qt5WebEngineWidgets_VERSION VERSION_LESS 5.9.3)
    message(WARNING "QWebEngine version ${Qt5WebEngineWidgets_VERSION} is known to be unstable with KMyMoney")
  endif()
else(ENABLE_WEBENGINE)
  find_package(KF5WebKit REQUIRED)
endif(ENABLE_WEBENGINE)

# check for optional LibOFX support
find_package(LibOfx)
if(LIBOFX_FOUND)
  option(ENABLE_OFXIMPORTER "Enable OFX Importer" ON)
  check_struct_has_member("struct OfxFiLogin" "clientuid" "libofx/libofx.h" LIBOFX_HAVE_CLIENTUID)
  if(NOT ENABLE_OFXIMPORTER)
    unset(LIBOFX_FOUND CACHE)
    unset(LIBOFX_HAVE_CLIENTUID CACHE)
  endif(NOT ENABLE_OFXIMPORTER)
else(LIBOFX_FOUND)
  option(ENABLE_OFXIMPORTER "Enable OFX Importer" OFF)
  if(ENABLE_OFXIMPORTER)
    unset(LIBOFX_FOUND CACHE)
    unset(LIBOFX_HAVE_CLIENTUID CACHE)
  endif(ENABLE_OFXIMPORTER)
endif(LIBOFX_FOUND)

# check for optional KBanking support
set(KBANKING_FOUND "AUTO")
mark_as_advanced(KBANKING_FOUND)
if(DEFINED ENABLE_KBANKING)
  set(KBANKING_FOUND OFF)
endif(DEFINED ENABLE_KBANKING)
option(ENABLE_KBANKING "Enable KBanking plugin" ON)
if(ENABLE_KBANKING)
  find_package(Qt5QuickWidgets) # Includes Qt5Qml

  find_package(AQBANKING 5.6.5)
  find_package(gwenhywfar 4.15.3)
  find_package(gwengui-cpp)
  find_package(gwengui-qt5)
  if (AQBANKING_FOUND AND gwengui-cpp_FOUND AND gwengui-qt5_FOUND AND Qt5QuickWidgets_FOUND)
    set(KBANKING_FOUND ON)
  else()
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
  if(KF5Kross_FOUND)
    set(WEBOOB_FOUND ON)
  else(KF5Kross_FOUND)
    if(NOT WEBOOB_FOUND STREQUAL "AUTO")
      message(FATAL_ERROR "Weboob requirements not met")
    endif(NOT WEBOOB_FOUND STREQUAL "AUTO")
    set(WEBOOB_FOUND OFF)
    set(ENABLE_WEBOOB OFF CACHE BOOL "Enable weboob plugin" FORCE)
  endif(KF5Kross_FOUND)
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

option(ENABLE_QIFIMPORTER "Enable QIF Importer" ON)
option(ENABLE_QIFEXPORTER "Enable QIF Exporter" ON)

option(ENABLE_GNCIMPORTER "Enable GNC Importer" ON)

option(ENABLE_CSVIMPORTER "Enable CSV Importer" ON)
option(ENABLE_CSVEXPORTER "Enable CSV Exporter" ON)

option(ENABLE_UNFINISHEDFEATURES "For devs only" OFF)

# TODO: this should be removed
enable_testing()

######################### Settings ##########################

# If no build type is set, use "Release with Debug Info"
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
  "Choose the type of build.
Possible values are: 'Release' 'RelWithDebInfo' 'Debug' 'DebugKMM' 'Debugfull' 'Profile'
The default value is: 'RelWithDebInfo'" FORCE)

# tells gcc to enable exception handling
include(KDECompilerSettings)
kde_enable_exceptions()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--as-needed")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
endif()

# IDEA: Set on a per target base
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # be more pedantic about common symbols
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-common -Wextra")

  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(IS_GNU 1)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wlogical-op")
  endif()

  # DebugKMM, Debugfull, Profile
  set(CMAKE_CXX_FLAGS_DEBUGKMM
    "-g -O2 -fno-reorder-blocks -fno-schedule-insns -fno-inline")
  set(CMAKE_CXX_FLAGS_DEBUGFULL
    "-g3 -fno-inline")
  set(CMAKE_CXX_FLAGS_PROFILE
    "-g3 -fno-inline -ftest-coverage -fprofile-arcs")

  # be pedantic about undefined symbols when linking shared libraries
  if(CMAKE_SYSTEM_NAME MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "kFreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "GNU")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined -Wl,--as-needed")
  endif()
endif()

# preprocessor definitions in case this is a debug build
set(CMAKE_CXX_FLAGS_DEBUGFULL "${CMAKE_CXX_FLAGS_DEBUGFULL} -DQT_STRICT_ITERATORS -DKMM_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUGKMM  "${CMAKE_CXX_FLAGS_DEBUGFULL} -DKMM_DEBUG")

option(USE_MODELTEST
  "Compile with ModelTest code (default=OFF)" OFF)

option(USE_QT_DESIGNER
  "Install KMyMoney specific widget library for Qt-Designer (default=OFF)" OFF)

######################### The Actual Targets ##########################
add_subdirectory( libkgpgfile )
add_subdirectory( tools )
add_subdirectory( kmymoney )
if(KF5DocTools_FOUND)
  add_subdirectory( doc )
endif()

######################### Output Results #############################

# create the config.h file out of the config.h.cmake
configure_file("config-kmymoney.h.cmake" "${CMAKE_CURRENT_BINARY_DIR}/config-kmymoney.h")
configure_file("config-kmymoney-version.h.cmake" "${CMAKE_CURRENT_BINARY_DIR}/config-kmymoney-version.h")

# this macro maps the boolean variable ${_varname} to "yes"/"no"
# and writes the output to the variable nice_${_varname}
macro(nice_yesno _varname)
  if(${_varname})
    set("nice_${_varname}" "yes")
  else(${_varname})
    set("nice_${_varname}" "no")
  endif(${_varname})
endmacro()

nice_yesno("KF5Holidays_FOUND")
nice_yesno("Gpgmepp_FOUND")
nice_yesno("KMM_ADDRESSBOOK_FOUND")
nice_yesno("LIBOFX_FOUND")
nice_yesno("LIBOFX_HAVE_CLIENTUID")
nice_yesno("KBANKING_FOUND")
nice_yesno("WEBOOB_FOUND")
nice_yesno("LIBICAL_FOUND")
nice_yesno("ENABLE_SQLCIPHER")
nice_yesno("USE_QT_DESIGNER")
nice_yesno("USE_MODELTEST")
nice_yesno("DOXYGEN_FOUND")
nice_yesno("ENABLE_WEBENGINE")
message("
-------- KMyMoney ${PROJECT_VERSION}${PROJECT_VERSION_SUFFIX} --------

Configure results (user options):
--------------------------------------------
GpgME Encryption:                        ${GPG_ENCRYPTION}

KDE PIM holidays:                        ${nice_KF5Holidays_FOUND}

KDE PIM addressbook:                     ${nice_KMM_ADDRESSBOOK_FOUND}

OFX plugin:                              ${nice_LIBOFX_FOUND}
  OFX supports CLIENTUID:                ${nice_LIBOFX_HAVE_CLIENTUID}

KBanking plugin:                         ${nice_KBANKING_FOUND}

weboob plugin:                           ${nice_WEBOOB_FOUND}

iCalendar export plugin:                 ${nice_LIBICAL_FOUND}

SQLCipher plugin:                        ${nice_ENABLE_SQLCIPHER}

QWebEngine:                              ${nice_ENABLE_WEBENGINE}

--------------------------------------------


Configure results (developer options):
--------------------------------------------
Qt-Designer library support:             ${nice_USE_QT_DESIGNER}

Generate modeltest code:                 ${nice_USE_MODELTEST}

Generate API documentation with Doxygen: ${nice_DOXYGEN_FOUND}")

message("
Build type: ${CMAKE_BUILD_TYPE}")

