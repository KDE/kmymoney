# SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>

# For all the configurable CMake switches, please see https://invent.kde.org/office/kmymoney/-/wikis/home#cmake-switches

# The CMake version we require
cmake_minimum_required(VERSION 3.16)

option(BUILD_STATIC_PLUGINS "Build the plugins statically" OFF)

set(KF6_MIN_VERSION 6.1)
set(KF5_MIN_VERSION 5.90)
set(QT6_MIN_VERSION 6.7)
set(QT5_MIN_VERSION 5.15.2)
set(KDE5_MIN_LEVEL 5.82)
set(KDE6_MIN_LEVEL ${KF6_MIN_VERSION})

# adds option BUILD_WITH_QT6
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
include(QtVersionOption)
set(KF_MIN_VERSION ${KF${QT_MAJOR_VERSION}_MIN_VERSION})
set(QT_MIN_VERSION ${QT${QT_MAJOR_VERSION}_MIN_VERSION})
set(KDE_COMPILERSETTINGS_LEVEL ${KDE${QT_MAJOR_VERSION}_MIN_LEVEL})

if (NOT BUILD_STATIC_PLUGINS)
    set(AKONADI_MIN_VERSION 5.14.2)
endif()


set(RELEASE_SERVICE_VERSION_MAJOR "5")
set(RELEASE_SERVICE_VERSION_MINOR "1")
set(RELEASE_SERVICE_VERSION_MICRO "90")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

# Use ccache if available
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

# Setting the name of the main project
project(KMyMoney VERSION "${RELEASE_SERVICE_VERSION}" LANGUAGES CXX)

# Minimum versions required for extern packages
set(KMM_ALKIMIA_MIN_VERSION 8.1.90)
set(KMM_KCHART_MIN_VERSION 2.6.0)
# LibOFX uses two different version numbers. find_package looks for the
# library's so version whereas pkg_check_modules retrieves the package
# version. Details can be found in the libofx source code
set(KMM_LIBOFX_MIN_VERSION 0.10.0)
set(KMM_LIBOFX_MIN_SOVERSION 7.0.0)
set(KMM_AQBANKING_MIN_VERSION 6.5.0)
set(KMM_GWENHYWFAR_MIN_VERSION 5.10.1)

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
elseif(DEFINED VERSION_SUFFIX)
  string(REGEX REPLACE "-*(.+)" "\\1" DASHFREE_SUFFIX "${VERSION_SUFFIX}")
  set(PROJECT_VERSION_SUFFIX "-${DASHFREE_SUFFIX}")
  set(${PROJECT_NAME}_VERSION_SUFFIX "-${DASHFREE_SUFFIX}")
endif()

if (POLICY CMP0063)
  cmake_policy(SET CMP0063 NEW) # Policy introduced in CMake version 3.3
endif()
if (POLICY CMP0127)
  cmake_policy(SET CMP0127 NEW) # Policy introduced in CMake version 3.22
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

######################### General Requirements ##########################

if (CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.0.0)
    message(FATAL_ERROR "This version of KMyMoney requires at least gcc 6.0.0 to be built successfully")
endif()

# To build shared libraries in Windows, we set CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS to TRUE.
# See https://cmake.org/cmake/help/v3.4/variable/CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS.html
# See https://blog.kitware.com/create-dlls-on-windows-without-declspec-using-new-cmake-export-all-feature/
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(APPLE_SUPPRESS_X11_WARNING TRUE)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDEInstallDirs)
include(KDECMakeSettings)

include(FeatureSummary)
include(CMakeDependentOption)

include(GenerateExportHeader)
include(KMyMoneyMacros)

include(KDEGitCommitHooks)
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

if(BUILD_WITH_QT6)
    message(WARNING
      "Qt6/KF6 support is not fully tested, may not work in specific cases"
      " and may corrupt your data."
      " If you find an error, please open a bug report at"
      " https://bugs.kde.org/enter_bug.cgi?product=kmymoney."
    )
    if(NOT BUILD_WITH_QT6_CONFIRMED)
        message(FATAL_ERROR
            "Add -DBUILD_WITH_QT6_CONFIRMED to the"
            " cmake configure line to confirm that you have read the previous note."
        )
    endif()
    # allow QString("...")
    add_definitions(
        -UQT_NO_CAST_FROM_ASCII
        -UQT_NO_CAST_FROM_BYTEARRAY
        -UQT_NO_CAST_TO_ASCII
        -UQT_NO_CAST_TO_BYTEARRAY
    )
endif()

add_definitions(
    -DKF_DEPRECATED_WARNINGS_SINCE=0x60000
    -DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x56200C
    -DQT_DEPRECATED_WARNINGS_SINCE=0x60000
    -DQT_DISABLE_DEPRECATED_BEFORE=0x50f02
)

# GPG Encryption checks
# ---------------------
set(GPG_FOUND FALSE)
if(BUILD_WITH_QT6)
    find_package(QGpgmeQt6)
    if(QGpgmeQt6_FOUND)
        set(GPG_FOUND TRUE)
        set(GPG_DIR ${QGpgmeQt6_DIR})
        set(GPG_LIBRARY QGpgmeQt6)
        set(GPG_VERSION ${QGpgmeQt6_VERSION})
    endif()
else()
    find_package(QGpgme)
    if (QGpgme_FOUND)
        set(GPG_FOUND TRUE)
        set(GPG_DIR ${QGpgme_DIR})
        set(GPG_LIBRARY QGpgme)
        set(GPG_VERSION ${QGpgme_VERSION})
    else()
        find_package(Gpgmepp)
        if (Gpgmepp_FOUND)
            set(GPG_FOUND TRUE)
            set(GPG_DIR ${Gpgmepp_DIR})
            set(GPG_LIBRARY Gpgmepp)
            set(GPG_VERSION ${Gpgmepp_VERSION})
        endif()
    endif()
endif()
if(GPG_FOUND)
    message(STATUS "Found GPG: ${GPG_DIR} (version \"${GPG_VERSION}\")")
    find_path(GPGMEPP_HEADER_DIR "gpgme++/error.h" ${CMAKE_SYSTEM_INCLUDE_PATH})
    file(READ "${GPGMEPP_HEADER_DIR}/gpgme++/error.h" GPGMEPP_ERROR_H_CONTENT)
    string(FIND "${GPGMEPP_ERROR_H_CONTENT}" "std::string asStdString() const" FOUND_AS_STD_STRING)
    set(HAVE_GPGMEPP_AS_STD_STRING FALSE)
    if (${FOUND_AS_STD_STRING} GREATER 0)
        set(HAVE_GPGMEPP_AS_STD_STRING TRUE)
        message(STATUS "Found GpgME::Error::asStdString")
    endif()
endif()

cmake_dependent_option(ENABLE_GPG "Enable GPG support." ON
                       "GPG_FOUND" OFF)

add_feature_info("Encryption" ENABLE_GPG "Allows to store your financial data using strong GPG encryption.")





find_package(PkgConfig)
if (PkgConfig_FOUND)
  pkg_check_modules(SQLCIPHER sqlcipher IMPORTED_TARGET)
endif()

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED COMPONENTS Core DBus Widgets Svg Xml Test PrintSupport)
find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} OPTIONAL_COMPONENTS Sql Concurrent QuickWidgets)

find_package(KF${QT_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS Archive CoreAddons Config ConfigWidgets I18n Completion KCMUtils ItemModels ItemViews Service XmlGui TextWidgets Notifications KIO)

# For KF < 5.100 we need to fix a few method names that
# have been deprecated but are not available before that version.
# Once the KF_MIN_VERSION of this project is >= 5.100 this
# whole block can be removed
if (${KF_MIN_VERSION} VERSION_LESS "5.100")
    if (${KF5_VERSION} VERSION_LESS "5.100")
        add_definitions(
            -DquestionTwoActions=questionYesNo
            -DquestionTwoActionsCancel=questionYesNoCancel
            -DwarningTwoActions=warningYesNo
            -DwarningTwoActionsCancel=warningYesNoCancel
            -DPrimaryAction=Yes
            -DSecondaryAction=No
        )
    endif()
elseif(NOT BUILD_WITH_QT6)
    message(FATAL_ERROR "Remove the above logic from CMakeLists.txt since it is not needed anymore")
endif()

if(BUILD_WITH_QT6)
    find_package(PlasmaActivities)
    find_package(Qt6Core5Compat)
else()
    set(KF_ACTIVITIES Activities)
endif()
find_package(KF${QT_MAJOR_VERSION} ${KF_MIN_VERSION} OPTIONAL_COMPONENTS DocTools Holidays Contacts ${KF_ACTIVITIES})
if (DEFINED AKONADI_MIN_VERSION)
  # Akonadi 23.04 moved to KPim5 prefix, deprecating KF5
  set(PIMPREFIX KPim${QT_MAJOR_VERSION})
  find_package(${PIMPREFIX}Akonadi ${AKONADI_MIN_VERSION} QUIET)
  if(NOT ${PIMPREFIX}Akonadi_FOUND)
    set(PIMPREFIX KF${QT_MAJOR_VERSION})
    find_package(${PIMPREFIX}Akonadi ${AKONADI_MIN_VERSION} QUIET)
  endif()
  if(${PIMPREFIX}Akonadi_FOUND)
    # for config-kmymoney.h.cmake
    set(ADDRESSBOOK_VERSION ${${PIMPREFIX}Akonadi_VERSION})
  endif()
  find_package(${PIMPREFIX}IdentityManagement ${AKONADI_MIN_VERSION} QUIET)
endif()

find_package(Qt${QT_MAJOR_VERSION}Keychain CONFIG REQUIRED)

if(LibAlkimia${QT_MAJOR_VERSION}_DIR)
  set(_LibAlkimia${QT_MAJOR_VERSION}_DIR ${LibAlkimia${QT_MAJOR_VERSION}_DIR})
endif()
find_package(LibAlkimia${QT_MAJOR_VERSION} ${KMM_ALKIMIA_MIN_VERSION} REQUIRED)
if (NOT LibAlkimia${QT_MAJOR_VERSION}_FOUND)
  # restore LibAlkimia5_DIR set to NOTFOUND by first find_package call
  if(_LibAlkimia${QT_MAJOR_VERSION}_DIR)
    set(LibAlkimia${QT_MAJOR_VERSION}_DIR ${_LibAlkimia${QT_MAJOR_VERSION}_DIR})
  endif()
  find_package(LibAlkimia${QT_MAJOR_VERSION} 8.1 REQUIRED)
endif()

if(BUILD_WITH_QT6)
    find_package(KChart6 ${KMM_KCHART_MIN_VERSION} QUIET)
else()
    find_package(KChart ${KMM_KCHART_MIN_VERSION} QUIET)
endif()

# we want the lowercase 'libofx' package name, for 'libofx_FOUND' compatibility with what pkg-config returns,
# in case it is to be used as fallback
find_package(libofx ${KMM_LIBOFX_MIN_SOVERSION} NAMES LibOFX libofx)
if(NOT libofx_FOUND)
    find_package(PkgConfig)

    if(PkgConfig_FOUND)
        pkg_check_modules(libofx IMPORTED_TARGET libofx)

        if(libofx_FOUND)
            if (${libofx_VERSION} VERSION_GREATER_EQUAL ${KMM_LIBOFX_MIN_VERSION})
                add_library(libofx::libofx INTERFACE IMPORTED)
                target_link_libraries(libofx::libofx INTERFACE PkgConfig::libofx)
            else()
                message(STATUS "Version ${libofx_VERSION} of libofx does not meet min. required version ${KMM_LIBOFX_MIN_VERSION}")
                set(libofx_FOUND FALSE)
            endif()
        endif()
    endif()
endif()


add_definitions(-DQT_USE_QSTRINGBUILDER -DQT_NO_CAST_TO_ASCII -DQT_NO_URL_CAST_FROM_STRING -DQT_NO_KEYWORDS)
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

if(QT_MAJOR_VERSION LESS 6)
  # This is needed for QtSqlite and QtDesigner
  # (they'll install files to ${QT_INSTALL_DIR}/plugins/)
  get_filename_component(QT_BIN_DIR "${QT_MOC_EXECUTABLE}" PATH)
  get_filename_component(QT_DIR     ${QT_BIN_DIR}        PATH)
  set(QT_INSTALL_DIR ${QT_DIR} CACHE PATH
    "Qt install prefix defaults to the Qt prefix: ${QT_DIR}")
endif()

cmake_dependent_option(ENABLE_ADDRESSBOOK "Enable addressbook support." ON
                       "${PIMPREFIX}IdentityManagement_FOUND;${PIMPREFIX}Akonadi_FOUND;KF${QT_MAJOR_VERSION}Contacts_FOUND" OFF)

add_feature_info("Address book" ENABLE_ADDRESSBOOK "Allows fetching payee information from KDE PIM system.")

cmake_dependent_option(ENABLE_HOLIDAYS "Enable addressbook support." ON
                       "KF${QT_MAJOR_VERSION}Holidays_FOUND" OFF)

add_feature_info("Holidays" ENABLE_HOLIDAYS "Allows fetching holidays from KDE PIM system.")

cmake_dependent_option(ENABLE_ACTIVITIES "Enable activities support." ON
                       "KF${QT_MAJOR_VERSION}Activities_FOUND" OFF)

option(ENABLE_FORECASTVIEW "Enable forecast view" ON)
add_feature_info("Forecast view" ENABLE_FORECASTVIEW "Adds possibility to calculate forecasts.")
cmake_dependent_option(ENABLE_REPORTSVIEW "Enable reports view." ON
                       "KChart_FOUND OR KChart6_FOUND" OFF)
add_feature_info("Reports view" ENABLE_REPORTSVIEW "Adds possibility to display chart and table reports.")
option(ENABLE_BUDGETVIEW "Enable budget view" ON)
add_feature_info("Budget view" ENABLE_BUDGETVIEW "Adds possibility to plan a budget.")
option(ENABLE_ONLINEJOBOUTBOXVIEW "Enable online job outbox view" ON)
add_feature_info("Online job outbox view" ENABLE_ONLINEJOBOUTBOXVIEW "Adds outbox for sending online jobs.")

cmake_dependent_option(ENABLE_SQLSTORAGE "Enable SQL storage support." ON
                       "Qt${QT_MAJOR_VERSION}Sql_FOUND" OFF)

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
                       "Qt${QT_MAJOR_VERSION}Sql_FOUND" OFF)

add_feature_info("IBAN/BIC data" ENABLE_IBANBICDATA "Adds predefined IBAN/BIC numbers to KMyMoney (note: doesn't work yet).")

# check for optional KBanking support
find_package(aqbanking ${KMM_AQBANKING_MIN_VERSION} QUIET)
find_package(gwenhywfar ${KMM_GWENHYWFAR_MIN_VERSION} QUIET)
find_package(gwengui-cpp QUIET)
find_package(gwengui-qt${QT_MAJOR_VERSION} QUIET)

cmake_dependent_option(ENABLE_KBANKING "Enable KBanking plugin" ON
                       "AQBANKING_FOUND;gwengui-cpp_FOUND;gwengui-qt${QT_MAJOR_VERSION}_FOUND;Qt${QT_MAJOR_VERSION}QuickWidgets_FOUND" OFF)

add_feature_info(KBanking ENABLE_KBANKING "Interface for the following online banking protocols: HBCI, EBICS, OFX Direct Connect, Paypal")

# check for optional Woob support
find_package(Python3 COMPONENTS Interpreter Development)
set_package_properties(Python3 PROPERTIES
        TYPE OPTIONAL
        PURPOSE "Required by Woob plugin.")

cmake_dependent_option(ENABLE_WOOB "Enable Woob plugin" ON
                       "Python3_FOUND;Qt${QT_MAJOR_VERSION}Concurrent_FOUND" OFF)
add_feature_info(Woob ENABLE_WOOB "Online banking interface using Woob.")


# check for optional ical support
find_package(LibIcal)
cmake_dependent_option(ENABLE_LIBICAL "Enable Calendar plugin" ON
                       "LibIcal_FOUND" OFF)
add_feature_info(iCalendar ENABLE_LIBICAL "iCalendar integration.")

# Optional features currently under development
option(ENABLE_COSTCENTER "Enable cost center support" OFF)
add_feature_info("Cost center support" ENABLE_COSTCENTER "Adds support for cost centers (for devs only).")

# Setup translations
ki18n_install(po)

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

option(WARNINGS_AS_ERRORS "Force warnings to be reported as errors" OFF)
add_feature_info("Warnings as errors" WARNINGS_AS_ERRORS "Enforces all warnings to be reported as errors.")

# Set compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  add_compile_options(-Wall -Wextra -Wlogical-op -Wno-null-dereference -Wshadow -Wunused -Wno-misleading-indentation -Wsuggest-override -Wcast-qual -Wformat=2 -fno-common -Werror=return-type -Wzero-as-null-pointer-constant)

  if(WARNINGS_AS_ERRORS)
    add_compile_options(-Werror)
  endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
  add_compile_options(-Wall -Wextra -Wno-null-dereference -Wshadow -Wunused -Wno-misleading-indentation -Wsuggest-override -Wcast-qual -Wformat=2 -Wunreachable-code -fno-common -Werror=return-type -Wzero-as-null-pointer-constant)

  if(WARNINGS_AS_ERRORS)
    add_compile_options(-Werror)
  endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  add_compile_options(/experimental:external /external:anglebrackets /external:W0 /W3)

  if(WARNINGS_AS_ERRORS)
    add_compile_options(/WX)
  endif()
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
  set(CMAKE_CXX_FLAGS_DEBUGKMM  "${CMAKE_CXX_FLAGS_DEBUGKMM} -DKMM_DEBUG -DQT_FORCE_ASSERTS")
endif()

option(USE_MODELTEST
  "Compile with ModelTest code (default=OFF)" OFF)

add_feature_info("Model test" USE_MODELTEST "Generate modeltest code (for devs only).")

option(USE_QT_DESIGNER
  "Install KMyMoney specific widget library for Qt-Designer (default=OFF)" OFF)

add_feature_info("QtDesigner" USE_QT_DESIGNER "Qt-Designer library support (for devs only).")

option(KMM_I18N_GENERATION
  "Generate i18n messages and documentation (default=ON)" OFF)
add_feature_info("Internationalization" KMM_I18N_GENERATION "Generation of i18n files (messages and docs)")

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
if(KF${QT_MAJOR_VERSION}DocTools_FOUND)
  add_subdirectory( doc )
  if (${KMM_I18N_GENERATION})
    kdoctools_install( po )
  endif()
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
