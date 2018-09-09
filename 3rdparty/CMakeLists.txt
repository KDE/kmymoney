project (kmymoney-and-all-its-deps)

#
# Build all dependencies for KMyMoney and finally KMyMoney itself.
# Parameters: EXTERNALS_DOWNLOAD_DIR place to download all packages
#             INSTALL_ROOT place to install everything to
#
# Example usage: cmake ..\kmymoneydep -DEXTERNALS_DOWNLOAD_DIR=/dev2/d -DINSTALL_ROOT=/dev2/i


cmake_minimum_required(VERSION 2.8.6)

if(NOT SUBMAKE_JOBS)
	set(SUBMAKE_JOBS 1)
endif()

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(FATAL_ERROR "Compiling in the source directory is not supported. Use for example 'mkdir build; cd build; cmake ..'.")
endif (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)

# Tools must be obtained to work with:
include (ExternalProject)

# allow specification of a directory with pre-downloaded
# requirements
if(NOT IS_DIRECTORY ${EXTERNALS_DOWNLOAD_DIR})
    message(FATAL_ERROR "No externals download dir set. Use -DEXTERNALS_DOWNLOAD_DIR")
endif()

if(NOT IS_DIRECTORY ${INSTALL_ROOT})
    message(FATAL_ERROR "No install dir set. Use -DINSTALL_ROOT")
endif()

set(TOP_INST_DIR ${INSTALL_ROOT})
set(EXTPREFIX "${TOP_INST_DIR}")
set(CMAKE_PREFIX_PATH "${EXTPREFIX}")

message( STATUS "CMAKE_GENERATOR: ${CMAKE_GENERATOR}")

set(GLOBAL_BUILD_TYPE RelWithDebInfo)
set(GLOBAL_PROFILE ${GLOBAL_PROFILE} -DBUILD_TESTING=false)
# without --host=x86_64 gmp builds with optimizations for skylake and that prevents running KMyMoney on pre-skylake CPUs
set(GLOBAL_AUTOMAKE_PROFILE --host=x86_64-linux-gnu --disable-static --enable-shared)

set(SECURITY_EXE_LINKER_FLAGS "")
set(SECURITY_SHARED_LINKER_FLAGS "")
set(SECURITY_MODULE_LINKER_FLAGS "")

if (DEFINED EP_PREFIX)
	set_directory_properties(PROPERTIES EP_PREFIX ${EP_PREFIX})
endif ()


if (UNIX AND NOT APPLE)
	set(LINUX true)
    set(PATCH_COMMAND patch)
endif ()

# this list must be dependency-ordered
add_subdirectory( ext_iconv )
add_subdirectory( ext_lzma )
add_subdirectory( ext_icu )
add_subdirectory( ext_xml )
add_subdirectory( ext_gettext )
add_subdirectory( ext_png )
add_subdirectory( ext_boost )
add_subdirectory( ext_xslt )
add_subdirectory( ext_fontconfig)
add_subdirectory( ext_freetype)
add_subdirectory( ext_qt )
add_subdirectory( ext_phonon )
add_subdirectory( ext_frameworks )
add_subdirectory( ext_sqlite )
add_subdirectory( ext_ncurses )
add_subdirectory( ext_mysql )
add_subdirectory( ext_grantlee )
add_subdirectory( ext_applications )
add_subdirectory( ext_gmp )
add_subdirectory( ext_alkimia )
add_subdirectory( ext_kdiagram )
add_subdirectory( ext_ktoblzcheck )
add_subdirectory( ext_gwenhywfar )
add_subdirectory( ext_xmlsec1 )
add_subdirectory( ext_aqbanking )
add_subdirectory( ext_gpgerror )
add_subdirectory( ext_assuan2 )
add_subdirectory( ext_gpgme )
add_subdirectory( ext_tcl )
add_subdirectory( ext_sqlcipher )
add_subdirectory( ext_opensp )
add_subdirectory( ext_ofx )
add_subdirectory( ext_ical )
