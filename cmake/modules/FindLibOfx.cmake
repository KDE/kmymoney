# - Try to find LibOfx
# Once done this will define
#
#  LIBOFX_FOUND - system has LibOfx
#  LIBOFX_INCLUDE_DIR - the LibOfx include directory
#  LIBOFX_LIBRARIES - Link these to LibOfx
#  LIBOFX_DEFINITIONS - Compiler switches required for using LibOfx

# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
# Copied from FindLibXslt.cmake, 2009, Guillaume DE BURE, <gdebure@yahoo.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


IF (LIBOFX_INCLUDE_DIR AND LIBOFX_LIBRARIES)
   # in cache already
   SET(LibOfx_FIND_QUIETLY TRUE)
ENDIF (LIBOFX_INCLUDE_DIR AND LIBOFX_LIBRARIES)

IF (NOT WIN32)
   # use pkg-config to get the directories and then use these values
   # in the FIND_PATH() and FIND_LIBRARY() calls
   find_package(PkgConfig)
   pkg_check_modules(PC_OFX libofx)
   SET(LIBOFX_DEFINITIONS ${PC_OFX_CFLAGS_OTHER})
ENDIF (NOT WIN32)

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

IF (LIBOFX_FOUND)
   IF (NOT LibOfx_FIND_QUIETLY)
      MESSAGE(STATUS "Found LibOfx: ${LIBOFX_LIBRARIES}")
   ENDIF (NOT LibOfx_FIND_QUIETLY)
ELSE (LIBOFX_FOUND)
   IF (LibOfx_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could NOT find LibOfx")
   ENDIF (LibOfx_FIND_REQUIRED)
ENDIF (LIBOFX_FOUND)

MARK_AS_ADVANCED(LIBOFX_INCLUDE_DIR  LIBOFX_LIBRARIES )

