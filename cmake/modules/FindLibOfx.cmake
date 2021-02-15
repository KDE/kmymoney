# - Try to find LibOfx
# Once done this will define
#
#  LIBOFX_FOUND - system has LibOfx
#  LIBOFX_INCLUDE_DIR - the LibOfx include directory
#  LIBOFX_LIBRARIES - Link these to LibOfx
#  LIBOFX_DEFINITIONS - Compiler switches required for using LibOfx

# SPDX-FileCopyrightText: 2006 Alexander Neundorf <neundorf@kde.org>
# Copied from FindLibXslt.cmake, 2009, Guillaume DE BURE, <gdebure@yahoo.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT LIBOFX_MIN_VERSION)
  set(LIBOFX_MIN_VERSION "0.9.4")
endif(NOT LIBOFX_MIN_VERSION)

IF (LIBOFX_INCLUDE_DIR AND LIBOFX_LIBRARIES)
   # in cache already
   SET(LIBOFX_FIND_QUIETLY TRUE)
ENDIF (LIBOFX_INCLUDE_DIR AND LIBOFX_LIBRARIES)

IF (NOT WIN32 AND NOT APPLE)
   # use pkg-config to get the directories and then use these values
   # in the FIND_PATH() and FIND_LIBRARY() calls
   FIND_PACKAGE(PkgConfig)

   # according to https://svnweb.freebsd.org/ports/head/finance/kmymoney/files/patch-cmake_modules_FindLibOfx.cmake?view=markup
   # FreeBSD needs a little different variable name here to setup the LibOFX package infrastructure for us.
   if (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
      PKG_CHECK_MODULES(PC_OFX libofx>=${LIBOFX_MIN_VERSION})
   else (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
      PKG_CHECK_MODULES(LIBOFX libofx>=${LIBOFX_MIN_VERSION})
   endif (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")

  FIND_PATH(LIBOFX_INCLUDE_DIR libofx/libofx.h
      PATHS
      ${LIBOFX_INCLUDE_DIRS}
      ${PC_OFX_INCLUDE_DIRS}
  )

ELSE (NOT WIN32 AND NOT APPLE)
  FIND_PATH(LIBOFX_INCLUDE_DIR libofx/libofx.h
      PATHS
      ${PC_OFX_INCLUDEDIR}
      ${PC_OFX_INCLUDE_DIRS}
    )

  FIND_LIBRARY(LIBOFX_LIBRARIES NAMES ofx libofx
      PATHS
      ${PC_OFX_LIBDIR}
      ${PC_OFX_LIBRARY_DIRS}
    )

  IF (LIBOFX_INCLUDE_DIR AND LIBOFX_LIBRARIES)
    SET(LIBOFX_FOUND TRUE)
  ELSE (LIBOFX_INCLUDE_DIR AND LIBOFX_LIBRARIES)
    SET(LIBOFX_FOUND FALSE)
  ENDIF (LIBOFX_INCLUDE_DIR AND LIBOFX_LIBRARIES)
ENDIF (NOT WIN32 AND NOT APPLE)

IF (LIBOFX_FOUND)
   IF (NOT LIBOFX_FIND_QUIETLY)
      MESSAGE(STATUS "Found LibOfx: ${LIBOFX_LIBRARY_DIRS}")
   ENDIF (NOT LIBOFX_FIND_QUIETLY)
ELSE (LIBOFX_FOUND)
   IF (LibOfx_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could NOT find LibOfx")
   ENDIF (LibOfx_FIND_REQUIRED)
ENDIF (LIBOFX_FOUND)

# since on Windows for now libofx is a static library we need to add libopensp and libiconv (just like when linking libofx)
IF (WIN32)
    SET(OPENSP_FOUND FALSE)
    FIND_PATH(OPENSP_INCLUDES ParserEventGeneratorKit.h
        $ENV{KDEROOT}/include/opensp
    )

    FIND_LIBRARY(OPENSP_LIBRARIES
        NAMES sp133 libosp
        PATHS
            $ENV{KDEROOT}/lib
    )

    FIND_LIBRARY(ICONV_LIBRARIES iconv)

    IF (OPENSP_INCLUDES AND OPENSP_OPENSP_LIBRARIES)
        set(OPENSP_FOUND TRUE)
    ENDIF (OPENSP_INCLUDES AND OPENSP_OPENSP_LIBRARIES)

  IF (OPENSP_FOUND)
    IF (NOT OPENSP_FIND_QUIETLY)
      MESSAGE(STATUS "Found OPENSP library: ${OPENSP_LIBRARIES}")
    ENDIF (NOT OPENSP_FIND_QUIETLY)

  ELSE (OPENSP_FOUND)
    IF (OPENSP_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could NOT find OPENSP library\nPlease install it first")
    ENDIF (OPENSP_FIND_REQUIRED)
  ENDIF (OPENSP_FOUND)
  SET(LIBOFX_LIBRARIES ${LIBOFX_LIBRARIES} ${OPENSP_LIBRARIES} ${ICONV_LIBRARIES})
ENDIF (WIN32)

MARK_AS_ADVANCED(LIBOFX_INCLUDE_DIR  LIBOFX_LIBRARIES )

