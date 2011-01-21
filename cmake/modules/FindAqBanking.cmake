# Find AqBanking
#
#  AQBANKING_FOUND - system has AqBanking with the minimum version needed
#  AQBANKING_INCLUDE_DIRS - the AqBanking include directories
#  AQBANKING_LIBRARIES - The libraries needed to use AqBanking
#  AQBANKING_VERSION = The version of AqBanking as defined in version.h

set(AQBANKING_FOUND FALSE)

if(NOT AQBANKING_MIN_VERSION)
  set(AQBANKING_MIN_VERSION "5.0.0")
endif(NOT AQBANKING_MIN_VERSION)

if(NOT AQBANKING_MAX_VERSION)
  # for some unknown reason, we need to give a micro version number
  # with an offset of 1 to the PKG_CHECK_MODULES macro.
  # The actual version of KBanking will work with is 4.99.8

  # set(AQBANKING_MAX_VERSION "4.99.9")
  
  # Currently there is no max version necessary
endif(NOT AQBANKING_MAX_VERSION)

if(AQBANKING_INCLUDE_DIRS AND AQBANKING_LIBRARIES)
  # Already in cache, be silent
  set(AQBANKING_FIND_QUIETLY TRUE)
endif(AQBANKING_INCLUDE_DIRS AND AQBANKING_LIBRARIES)

if(AQBANKING_MIN_VERSION AND AQBANKING_MAX_VERSION)
  PKG_CHECK_MODULES(AQBANKING aqbanking>=${AQBANKING_MIN_VERSION} aqbanking<=${AQBANKING_MAX_VERSION})
else (AQBANKING_MIN_VERSION AND AQBANKING_MAX_VERSION)
  PKG_CHECK_MODULES(AQBANKING aqbanking>=${AQBANKING_MIN_VERSION})
endif (AQBANKING_MIN_VERSION AND AQBANKING_MAX_VERSION)

mark_as_advanced(AQBANKING_INCLUDE_DIRS AQBANKING_LIBRARIES)
