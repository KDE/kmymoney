# Find AqBanking
#
#  AQBANKING_FOUND - system has AqBanking with the minimum version needed
#  AQBANKING_INCLUDE_DIRS - the AqBanking include directories
#  AQBANKING_LIBRARIES - The libraries needed to use AqBanking
#  AQBANKING_VERSION = The version of AqBanking as defined in version.h

include(MacroEnsureVersion)

set(AQBANKING_FOUND FALSE)

if(NOT AQBANKING_MIN_VERSION)
  set(AQBANKING_MIN_VERSION "4.1.8")
endif(NOT AQBANKING_MIN_VERSION)

if (WIN32)
  file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" _program_FILES_DIR)
endif(WIN32)

set(AQBANKING_FIND_REQUIRED ${AqBanking_FIND_REQUIRED})
if(AQBANKING_INCLUDE_DIRS AND AQBANKING_LIBRARIES)

  # Already in cache, be silent
  set(AQBANKING_FIND_QUIETLY TRUE)

endif(AQBANKING_INCLUDE_DIRS AND AQBANKING_LIBRARIES)

#set the root from the AQBANKING_BASE environment
file(TO_CMAKE_PATH "$ENV{AQBANKING_BASE}" aqbanking_root )
#override the root from AQBANKING_BASE defined to cmake
if(DEFINED AQBANKING_BASE)
  file(TO_CMAKE_PATH "${AQBANKING_BASE}" aqbanking_root )
endif(DEFINED AQBANKING_BASE)

find_path(AQBANKING_INCLUDE_DIR NAMES aqbanking/version.h
  HINTS ${aqbanking_root}/include /usr/local/qt4/include /usr/local/include ${_program_FILES_DIR}/aqbanking/include
)

string(REPLACE /include "" AQBASEDIR ${AQBANKING_INCLUDE_DIR})

find_library(AQBANKING_LIBRARY NAMES aqbanking libaqbanking
  HINTS ${AQBASEDIR}/lib
)

find_library(QBANKING_LIBRARY NAMES q4banking libq4banking
  HINTS ${AQBASEDIR}/lib
)

set(AQBANKING_INCLUDE_DIRS ${AQBANKING_INCLUDE_DIR})
set(AQBANKING_LIBRARIES ${AQBANKING_LIBRARY} ${QBANKING_LIBRARY})

if(AQBANKING_INCLUDE_DIRS AND AQBANKING_LIBRARIES)
  execute_process(COMMAND ${AQBASEDIR}/bin/aqbanking-config --vstring OUTPUT_VARIABLE AQBANKING_VERSION RESULT_VARIABLE RUN_RESULT)
  if(RUN_RESULT EQUAL 0)
    # get rid of trailing \n
    string(LENGTH ${AQBANKING_VERSION} LEN)
    math(EXPR LEN ${LEN}-1)
    string(SUBSTRING ${AQBANKING_VERSION} 0 ${LEN} AQBANKING_VERSION)
    message(STATUS "Found AqBanking version '${AQBANKING_VERSION}' in ${AQBASEDIR}")
    macro_ensure_version2(${AQBANKING_MIN_VERSION} ${AQBANKING_VERSION} AQBANKING_VERSION_OK)
    if(NOT AQBANKING_VERSION_OK)
      message(STATUS "AqBanking version ${AQBANKING_VERSION} is too old. At least version ${AQBANKING_MIN_VERSION} is needed.")
      set(AQBANKING_INCLUDE_DIRS "")
      set(AQBANKING_LIBRARIES "")
    else(NOT AQBANKING_VERSION_OK)
      set(AQBANKING_FOUND TRUE)
    endif(NOT AQBANKING_VERSION_OK)
  endif(RUN_RESULT EQUAL 0)
else(AQBANKING_INCLUDE_DIRS AND AQBANKING_LIBRARIES)
  message(STATUS "AqBanking headers not found. If you do have")
  message(STATUS "  the AqBanking development package installed use")
  message(STATUS "  the AQBANKING_BASE cmake variable to point to")
  message(STATUS "  the location where AqBanking is installed.")
endif(AQBANKING_INCLUDE_DIRS AND AQBANKING_LIBRARIES)

mark_as_advanced(AQBANKING_INCLUDE_DIRS AQBANKING_LIBRARIES)
