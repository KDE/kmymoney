# Find AqBanking
#
#  AQBANKING_FOUND - system has AqBanking with the minimum version needed
#  AQBANKING_INCLUDE_DIRS - the AqBanking include directories
#  AQBANKING_LIBRARIES - The libraries needed to use AqBanking
#  AQBANKING_VERSION = The version of AqBanking as defined in version.h

set(AQBANKING_FOUND FALSE)

if(NOT AQBANKING_MIN_VERSION)
  set(AQBANKING_MIN_VERSION "4.2.4")
endif(NOT AQBANKING_MIN_VERSION)

if(NOT AQBANKING_MAX_VERSION)
  # for some unknown reason, we need to give a micro version number
  # with an offset of 1 to the PKG_CHECK_MODULES macro. The actual
  # version KBanking will work with is 4.99.8
  set(AQBANKING_MAX_VERSION "4.99.9")
endif(NOT AQBANKING_MAX_VERSION)

if(AQBANKING_INCLUDE_DIRS AND AQBANKING_LIBRARIES)
  # Already in cache, be silent
  set(AQBANKING_FIND_QUIETLY TRUE)
endif(AQBANKING_INCLUDE_DIRS AND AQBANKING_LIBRARIES)

PKG_CHECK_MODULES(AQBANKING aqbanking>=${AQBANKING_MIN_VERSION} aqbanking<=${AQBANKING_MAX_VERSION})

if(${AQBANKING_FOUND})
  # if AqBanking has been found make sure to add the q4banking lib
  set(AQBANKING_LIBRARIES ${AQBANKING_LIBRARIES} q4banking)
endif(${AQBANKING_FOUND})

mark_as_advanced(AQBANKING_INCLUDE_DIRS AQBANKING_LIBRARIES)
