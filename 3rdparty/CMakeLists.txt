project (kmymoney-and-all-its-deps)

#
# Build all dependencies for KMyMoney and finally KMyMoney itself.
# Parameters: EXT_DOWNLOAD_DIR place to download all packages
#             INSTALL_ROOT place to install everything to
#
# Example usage: cmake ..\kmymoneydep -DEXT_DOWNLOAD_DIR=/dev2/d -DINSTALL_ROOT=/dev2/i


cmake_minimum_required(VERSION 2.8.6)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(FATAL_ERROR "Compiling in the source directory is not supported. Use for example 'mkdir build; cd build; cmake ..'.")
endif()

# Tools must be obtained to work with:
include(ExternalProject)

# allow specification of a directory with pre-downloaded
# requirements
if(NOT IS_DIRECTORY ${EXT_DOWNLOAD_DIR})
    message(FATAL_ERROR "No externals download dir set. Use -DEXT_DOWNLOAD_DIR")
endif()

if(NOT IS_DIRECTORY ${EXT_INSTALL_DIR})
    message(FATAL_ERROR "No install dir set. Use -DEXT_INSTALL_DIR")
endif()

message( STATUS "CMAKE_GENERATOR: ${CMAKE_GENERATOR}")
set(CMAKE_PREFIX_PATH "${EXT_INSTALL_DIR}")
set(CMAKE_INSTALL_PREFIX "${EXT_INSTALL_DIR}")
set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(CMAKE_GLOBAL_PROFILE
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
  -DCMAKE_INSTALL_PREFIX=${EXT_INSTALL_DIR}
  -DCMAKE_PREFIX_PATH=${EXT_INSTALL_DIR}
  -DBUILD_TESTING=FALSE
)

# without --host=x86_64 gmp builds with optimizations for skylake and that prevents running KMyMoney on pre-skylake CPUs
set(AT_HOST --host=x86_64-linux-gnu)
set(AT_SHARED_ONLY --disable-static --enable-shared)
set(AT_CPP_FLAGS "-L${EXT_INSTALL_DIR}/lib -I${EXT_INSTALL_DIR}/include")

set(PATCH_COMMAND patch)

# this list must be dependency-ordered
add_subdirectory( ext_iconv )
add_subdirectory( ext_zlib )
add_subdirectory( ext_lzma )
add_subdirectory( ext_icu )
add_subdirectory( ext_ncurses )
add_subdirectory( ext_readline )
add_subdirectory( ext_xml )
add_subdirectory( ext_gettext )
add_subdirectory( ext_png )
add_subdirectory( ext_nasm )
add_subdirectory( ext_jpeg )
add_subdirectory( ext_boost )
add_subdirectory( ext_xslt )
add_subdirectory( ext_fontconfig )
add_subdirectory( ext_intltool )
add_subdirectory( ext_sharedmimeinfo )
add_subdirectory( ext_gmp )
add_subdirectory( ext_gpgerror )
add_subdirectory( ext_assuan2 )
add_subdirectory( ext_nettle )
add_subdirectory( ext_tasn1 )
add_subdirectory( ext_unistring )
add_subdirectory( ext_gnutls )
add_subdirectory( ext_pcre2 )
add_subdirectory( ext_freetype )
add_subdirectory( ext_harfbuzz )
add_subdirectory( ext_sqlite )
add_subdirectory( ext_openssl )
add_subdirectory( ext_mysql )
add_subdirectory( ext_tcl )
add_subdirectory( ext_postgresql )
add_subdirectory( ext_qt )
add_subdirectory( ext_phonon )
add_subdirectory( ext_frameworks )
add_subdirectory( ext_grantlee )
add_subdirectory( ext_applications )
add_subdirectory( ext_alkimia )
add_subdirectory( ext_kdiagram )
add_subdirectory( ext_ktoblzcheck )
add_subdirectory( ext_gcrypt )
add_subdirectory( ext_gwenhywfar )
add_subdirectory( ext_xmlsec1 )
add_subdirectory( ext_aqbanking )
add_subdirectory( ext_gpgme )
add_subdirectory( ext_sqlcipher )
add_subdirectory( ext_opensp )
add_subdirectory( ext_ofx )
add_subdirectory( ext_ical )
