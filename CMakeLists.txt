# SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>

# For all the configurable CMake switches, please see https://invent.kde.org/office/kmymoney/-/wikis/home#cmake-switches

# The CMake version we require
cmake_minimum_required(VERSION 3.16)

option(BUILD_STATIC_PLUGINS "Build the plugins statically" OFF)

set(KF5_MIN_VERSION 5.90)
set(QT_MIN_VERSION 5.15.2)
set(KDE_COMPILERSETTINGS_LEVEL 5.82)

if (NOT BUILD_STATIC_PLUGINS)
    set(AKONADI_MIN_VERSION 5.14.2)
endif()


set(KMYMONEY_STABLE_BRANCH TRUE)
set(RELEASE_SERVICE_VERSION_MAJOR "5")
set(RELEASE_SERVICE_VERSION_MINOR "1")
set(RELEASE_SERVICE_VERSION_MICRO "80")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

# Setting the name of the main project
project(KMyMoney VERSION "${RELEASE_SERVICE_VERSION}" LANGUAGES CXX)

# Determine the GIT reference (if we're based on GIT)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
  execute_process(COMMAND git rev-parse --short HEAD
          WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
          OUTPUT_VARIABLE VERSION_SUFFIX
          OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(VERSION_SUFFIX "-${VERSION_SUFFIX}")
  # Add variables which are similar to the build in names of cmake
  set(PROJECT_VERSION_SUFFIX "${VERSION_SUFFIX}")
  set(${PROJECT_NAME}_VERSION_SUFFIX "${VERSION_SUFFIX}")

  execute_process(
          COMMAND git rev-parse --abbrev-ref HEAD
          WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
          OUTPUT_VARIABLE GIT_BRANCH
          OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(TOLOWER "${GIT_BRANCH}" GIT_BRANCH)
  message(STATUS "Working on branch ${GIT_BRANCH}")
  if ("${GIT_BRANCH}" MATCHES "master|main")
      set(KMYMONEY_STABLE_BRANCH FALSE)
      message(STATUS "Detected an unstable ${GIT_BRANCH} branch")
  endif()
elseif(DEFINED VERSION_SUFFIX)
  string(REGEX REPLACE "-*(.+)" "\\1" DASHFREE_SUFFIX "${VERSION_SUFFIX}")
  set(PROJECT_VERSION_SUFFIX "-${DASHFREE_SUFFIX}")
  set(${PROJECT_NAME}_VERSION_SUFFIX "-${DASHFREE_SUFFIX}")
endif()

if (POLICY CMP0063)
  cmake_policy(SET CMP0063 NEW) # Policy introduced in CMake version 3.3
endif()
if (POLICY CMP0071)
  # We do not require the old behaviour. It is only set to old, to prevent accidental use of
  # the new behaviour. If the new behaviour becomes important, cmake_minimum_required has to be
  # set to "3.10".
  cmake_policy(SET CMP0071 OLD) # Policy introduced in CMake version 3.10
endif()


######################### General Requirements ##########################

if (CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.0.0)
    message(FATAL_ERROR "This version of KMyMoney requires at least gcc 6.0.0 to be built successfully")
endif()

# To build shared libraries in Windows, we set CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS to TRUE.
# See https://cmake.org/cmake/help/v3.4/variable/CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS.html
# See https://blog.kitware.com/create-dlls-on-windows-without-declspec-using-new-cmake-export-all-feature/
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(APPLE_SUPPRESS_X11_WARNING TRUE)
find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(KDEInstallDirs)
if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/po)
    # Block KDE's translation infrastructure, we'll use our own
    add_custom_target(fetch-translations)
endif()
include(KDECMakeSettings)

include(FeatureSummary)
include(CMakeDependentOption)

include(GenerateExportHeader)
include(KMyMoneyMacros)

include(KDEGitCommitHooks)
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

add_definitions(
    -DKF_DEPRECATED_WARNINGS_SINCE=0x60000
    -DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x56200C
    -DQT_DEPRECATED_WARNINGS_SINCE=0x60000
    -DQT_DISABLE_DEPRECATED_BEFORE=0x50f02
)

find_package(PkgConfig)

find_package(Gpgmepp)
if (Gpgmepp_FOUND)
  set(GPG_FOUND TRUE)
else()
  set(GPG_FOUND FALSE)
  find_package(KF5 ${KF5_MIN_VERSION} OPTIONAL_COMPONENTS QGpgme)
endif()

if (PkgConfig_FOUND)
  pkg_check_modules(SQLCIPHER sqlcipher IMPORTED_TARGET)
endif()

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core DBus Widgets Svg Xml Test PrintSupport)
find_package(Qt5 ${QT_MIN_VERSION} OPTIONAL_COMPONENTS Sql Concurrent QuickWidgets WebEngineWidgets WebKitWidgets)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS Archive CoreAddons Config ConfigWidgets I18n Completion KCMUtils ItemModels ItemViews Service Wallet XmlGui TextWidgets Notifications KIO)
find_package(KF5 ${KF5_MIN_VERSION} OPTIONAL_COMPONENTS DocTools Holidays Contacts Activities)
if (DEFINED AKONADI_MIN_VERSION)
  find_package(KF5 ${AKONADI_MIN_VERSION} OPTIONAL_COMPONENTS Akonadi IdentityManagement)
endif()

if(LibAlkimia5_DIR)
  set(_LibAlkimia5_DIR ${LibAlkimia5_DIR})
endif()
find_package(LibAlkimia5 8.0)
if (NOT LibAlkimia5_FOUND)
  # restore LibAlkimia5_DIR set to NOTFOUND by first find_package call
  if(_LibAlkimia5_DIR)
    set(LibAlkimia5_DIR ${_LibAlkimia5_DIR})
  endif()
  find_package(LibAlkimia5 7.0 REQUIRED)
endif()

find_package(KChart 2.6.0)

find_package(libofx)
if(NOT libofx_FOUND)
    find_package(PkgConfig)

    if(PkgConfig_FOUND)
        pkg_check_modules(libofx IMPORTED_TARGET libofx)

        if(libofx_FOUND)
            add_library(libofx::libofx INTERFACE IMPORTED)
            target_link_libraries(libofx::libofx INTERFACE PkgConfig::libofx)
        endif()
    endif()
endif()

if(KF5Gpgmepp_FOUND)
  set(GPG_FOUND TRUE)
endif()

cmake_dependent_option(ENABLE_GPG "Enable GPG support." ON
                       "GPG_FOUND" OFF)

add_feature_info("Encryption" ENABLE_GPG "Allows to store your financial data using strong GPG encryption.")

add_definitions(-DQT_USE_QSTRINGBUILDER -DQT_NO_CAST_TO_ASCII -DQT_NO_URL_CAST_FROM_STRING -DQT_NO_KEYWORDS)
add_definitions(-DTRANSLATION_DOMAIN=\"kmymoney\")
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
add_feature_info("Doxygen" DOXYGEN_FOUND "Generate API documentation with Doxygen (for devs only).")

# check some include files exists
set(CMAKE_REQUIRED_DEFINITIONS -D_XOPEN_SOURCE=500 -D_BSD_SOURCE)
include (CheckIncludeFileCXX)
check_include_file_cxx("unistd.h" HAVE_UNISTD_H)
check_include_file_cxx("pwd.h" HAVE_PWD_H)
check_include_file_cxx("windows.h" HAVE_WINDOWS_H)
check_include_file_cxx("lmcons.h" HAVE_LMCONS_H)
check_include_file_cxx("process.h" HAVE_PROCESS_H)

# include check for members in structs
include (CheckStructHasMember)

######################### Special Requirements ##########################

# This is needed for QtSqlite and QtDesigner
# (they'll install files to ${QT_INSTALL_DIR}/plugins/)
get_filename_component(QT_BIN_DIR "${QT_MOC_EXECUTABLE}" PATH)
get_filename_component(QT_DIR     ${QT_BIN_DIR}        PATH)
set(QT_INSTALL_DIR ${QT_DIR} CACHE PATH
  "Qt install prefix defaults to the Qt prefix: ${QT_DIR}")

cmake_dependent_option(ENABLE_ADDRESSBOOK "Enable addressbook support." ON
                       "KF5IdentityManagement_FOUND;KF5Akonadi_FOUND;KF5Contacts_FOUND" OFF)

add_feature_info("Address book" ENABLE_ADDRESSBOOK "Allows fetching payee information from KDE PIM system.")

cmake_dependent_option(ENABLE_HOLIDAYS "Enable addressbook support." ON
                       "KF5Holidays_FOUND" OFF)

add_feature_info("Holidays" ENABLE_HOLIDAYS "Allows fetching holidays from KDE PIM system.")

cmake_dependent_option(ENABLE_ACTIVITIES "Enable activities support." ON
                       "KF5Activities_FOUND" OFF)

option(ENABLE_FORECASTVIEW "Enable forecast view" ON)
add_feature_info("Forecast view" ENABLE_FORECASTVIEW "Adds possibility to calculate forecasts.")
cmake_dependent_option(ENABLE_REPORTSVIEW "Enable reports view." ON
                       "KChart_FOUND" OFF)
add_feature_info("Reports view" ENABLE_REPORTSVIEW "Adds possibility to display chart and table reports.")
option(ENABLE_BUDGETVIEW "Enable budget view" ON)
add_feature_info("Budget view" ENABLE_BUDGETVIEW "Adds possibility to plan a budget.")
option(ENABLE_ONLINEJOBOUTBOXVIEW "Enable online job outbox view" ON)
add_feature_info("Online job outbox view" ENABLE_ONLINEJOBOUTBOXVIEW "Adds outbox for sending online jobs.")

cmake_dependent_option(ENABLE_SQLSTORAGE "Enable SQL storage support." ON
                       "Qt5Sql_FOUND" OFF)

add_feature_info("SQL Storage" ENABLE_SQLSTORAGE "Allows storing your financial data in SQL database.")

cmake_dependent_option(ENABLE_SQLCIPHER "Enable SQLCipher support." ON
                       "SQLCIPHER_FOUND" OFF)

cmake_dependent_option(ENABLE_LIBOFX "Enable LibOFX support." ON "libofx_FOUND" OFF)
add_feature_info("LibOFX" ENABLE_LIBOFX "Enables OFX import, export, and Direct Connect using LibOFX library.")

cmake_dependent_option(ENABLE_SQLTRACER "Dump SQL requests to console" OFF
                       "ENABLE_SQLSTORAGE" OFF)

# Otherwise compilers halt on something like that:
# ld: library not found for -lsqlcipher
# on MS Windows, FreeBSD, macOS, and Ubuntu 14.04 (Ubuntu has CMake 3.5.1 but I'm not sure if it's the one to blame).
if(ENABLE_SQLCIPHER AND (NOT CMAKE_SYSTEM_NAME MATCHES "Linux" OR CMAKE_VERSION VERSION_LESS 3.5.2))
  link_directories(${SQLCIPHER_LIBRARY_DIRS})
endif()

add_feature_info("SQLCipher" ENABLE_SQLCIPHER "Allows encrypting your SQLite3 database.")

add_feature_info("SQL Tracer" ENABLE_SQLTRACER "It traces the SQL queries to the console.")

cmake_dependent_option(ENABLE_IBANBICDATA "Enable IBAN/BIC data support." OFF
                       "Qt5Sql_FOUND" OFF)

add_feature_info("IBAN/BIC data" ENABLE_IBANBICDATA "Adds predefined IBAN/BIC numbers to KMyMoney (note: doesn't work yet).")

# check for optional KBanking support
find_package(aqbanking 6.2.5)
find_package(gwenhywfar 5.4.1)
find_package(gwengui-cpp)
find_package(gwengui-qt5)

cmake_dependent_option(ENABLE_KBANKING "Enable KBanking plugin" ON
                       "AQBANKING_FOUND;gwengui-cpp_FOUND;gwengui-qt5_FOUND;Qt5QuickWidgets_FOUND" OFF)

add_feature_info(KBanking ENABLE_KBANKING "Interface for the following online banking protocols: HBCI, EBICS, OFX Direct Connect, Paypal")

# check for optional Woob support
find_package(Python3 COMPONENTS Interpreter Development)
set_package_properties(Python3 PROPERTIES
        TYPE OPTIONAL
        PURPOSE "Required by Woob plugin.")

cmake_dependent_option(ENABLE_WOOB "Enable Woob plugin" ON
                       "Python3_FOUND;Qt5Concurrent_FOUND" OFF)
add_feature_info(Woob ENABLE_WOOB "Online banking interface using Woob.")


# check for optional ical support
find_package(LibIcal)
cmake_dependent_option(ENABLE_LIBICAL "Enable Calendar plugin" ON
                       "LibIcal_FOUND" OFF)
add_feature_info(iCalendar ENABLE_LIBICAL "iCalendar integration.")

# Source translations (adapted from Krita)
find_package(Python3 COMPONENTS Interpreter)
set_package_properties(Python3 PROPERTIES
        TYPE OPTIONAL
        PURPOSE "Required to manually parse and fetch available translations.")

find_package(Subversion 1.4)
set_package_properties(Subversion PROPERTIES
        TYPE OPTIONAL
        PURPOSE "Required to access the KDE l10n SVN repository.")

cmake_dependent_option(FETCH_TRANSLATIONS "Retrieve translations before building KMyMoney" OFF
        "Python3_FOUND;Subversion_FOUND" OFF)
add_feature_info("Fetch translations" FETCH_TRANSLATIONS "Downloads KMyMoney translations from KDE l10n SVN")

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/po AND FETCH_TRANSLATIONS)
    set(TRANSLATIONS ${CMAKE_CURRENT_BINARY_DIR}/po)
        if (KMYMONEY_STABLE_BRANCH)
            set(tr_EXTRA_FLAGS "")
        else()
            set(tr_EXTRA_FLAGS "--unstable")
        endif()

    get_filename_component(absolute_podir ${TRANSLATIONS} ABSOLUTE)
    string(MD5 pathmd5 ${absolute_podir})

    find_file (KMYMONEY_PO_SCRIPT copy_po.py PATHS ${CMAKE_CURRENT_SOURCE_DIR}/packaging REQUIRED)
    if (NOT TARGET fetch-${pathmd5})
        file(MAKE_DIRECTORY ${TRANSLATIONS})

        add_custom_target(fetch-${pathmd5}
                COMMENT "Retrieving translations..."
                COMMAND ${Python3_EXECUTABLE} ${KMYMONEY_PO_SCRIPT} ${tr_EXTRA_FLAGS}
                BYPRODUCTS ${TRANSLATIONS}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                )
    endif()
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/po OR EXISTS ${CMAKE_CURRENT_BINARY_DIR}/po )
    find_package(KF5I18n CONFIG REQUIRED)
    ki18n_install(po)
    if (TRANSLATIONS)
        add_dependencies(pofiles-${pathmd5} fetch-${pathmd5})
        add_dependencies(tsfiles-${pathmd5} fetch-${pathmd5})
    endif()
endif()

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

# Set linker flags

# There is no way to detect linker in cmake (see https://gitlab.kitware.com/cmake/cmake/issues/17596)
# and linkers aren't compatible with each other, so we need to assume something
if(CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-dead_strip -Wl,-undefined,error")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-dead_strip -Wl,-undefined,error -Wl,-mark_dead_strippable_dylib")
  set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    -Wl,-dead_strip -Wl,-undefined,error")

elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--as-needed")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed")
  # TODO: remove multiple definitions of payeeIdentifierLoader::createPayeeIdentifierFromSqlDatabase
  set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed -Wl,--allow-multiple-definition")
  # CI builds are crashing on FreeBSD with --no-undefined. Probably because -DECM_ENABLE_SANITIZERS='address'
  # more can be read on the following site https://bugs.freedesktop.org/show_bug.cgi?id=100120
  if(NOT CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--no-undefined")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    -Wl,--no-undefined")
  endif()

  # TODO: remove multiple definitions of payeeIdentifierLoader::hasItemEditDelegate
  if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--allow-multiple-definition")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--allow-multiple-definition")
  endif()

elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /FORCE:Multiple")
endif()

# Set compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wlogical-op -Wno-null-dereference -Wshadow -Wunused -Wno-misleading-indentation -Wsuggest-override -Wcast-qual -Wformat=2 -fno-common")

elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wcast-qual -Wformat=2 -Wunreachable-code -fno-common")

elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /experimental:external /external:anglebrackets /external:W0 /W3")
endif()


# IDEA: Set on a per target base
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
  # DebugKMM, Debugfull, Profile
  set(CMAKE_CXX_FLAGS_DEBUGKMM
      "-g -O2 -fno-reorder-blocks -fno-schedule-insns -fno-inline")
  set(CMAKE_CXX_FLAGS_DEBUGFULL
      "-g3 -fno-inline")
  set(CMAKE_CXX_FLAGS_PROFILE
      "-g3 -fno-inline -ftest-coverage -fprofile-arcs")

  # preprocessor definitions in case this is a debug build
  set(CMAKE_CXX_FLAGS_DEBUGFULL "${CMAKE_CXX_FLAGS_DEBUGFULL} -DQT_STRICT_ITERATORS -DKMM_DEBUG -DQT_FORCE_ASSERTS")
  set(CMAKE_CXX_FLAGS_DEBUGKMM  "${CMAKE_CXX_FLAGS_DEBUGFULL} -DKMM_DEBUG -DQT_FORCE_ASSERTS")
endif()

option(USE_MODELTEST
  "Compile with ModelTest code (default=OFF)" OFF)

add_feature_info("Model test" USE_MODELTEST "Generate modeltest code (for devs only).")

option(USE_QT_DESIGNER
  "Install KMyMoney specific widget library for Qt-Designer (default=OFF)" OFF)

add_feature_info("QtDesigner" USE_QT_DESIGNER "Qt-Designer library support (for devs only).")

######################### The Actual Targets ##########################
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
if(WIN32)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
endif()

add_subdirectory( tools )
add_subdirectory( kmymoney )
if(KF5DocTools_FOUND)
  add_subdirectory( doc )
endif()

######################### Output Results #############################

# create the config.h file out of the config.h.cmake
configure_file("config-kmymoney.h.cmake" "${CMAKE_CURRENT_BINARY_DIR}/config-kmymoney.h")
configure_file("config-kmymoney-version.h.cmake" "${CMAKE_CURRENT_BINARY_DIR}/config-kmymoney-version.h")

message("
Build type: ${CMAKE_BUILD_TYPE}")

feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND
                DESCRIPTION "The following REQUIRED packages have not been found:")

feature_summary(WHAT OPTIONAL_PACKAGES_NOT_FOUND
                DESCRIPTION "The following OPTIONAL packages have not been found:")

feature_summary(WHAT ENABLED_FEATURES
                DESCRIPTION "The following features have been enabled:")

feature_summary(WHAT DISABLED_FEATURES
                DESCRIPTION "The following features have been disabled:")
