install(PROGRAMS financequote.pl
        DESTINATION ${DATA_INSTALL_DIR}/kmymoney/misc)

set( kmm_utils_validators_SRCS
  charvalidator.cpp
  validators.cpp
)

set( kmm_utils_webconnect_SRCS
  webconnect.cpp
)

if( "${HAVE_UNISTD_H}" AND "${HAVE_PWD_H}" )
  set( kmm_utils_platformtools_SRCS
    platformtools_gnu.cpp
  )
elseif( "${HAVE_WINDOWS_H}" AND "${HAVE_LMCONS_H}" AND "${HAVE_PROCESS_H}" )
  set( kmm_utils_platformtools_SRCS
    platformtools_nognu.cpp
  )
else()
  message(FATAL_ERROR "make sure either windows.h and lmcons.h (on msvc platforms) or unistd.h and pwd.h (on other platforms) are present.")
endif()

set( kmm_utils_validators_HEADER
  charvalidator.h
  validators.h
)

set( kmm_utils_webconnect_HEADER
  webconnect.h
)

set( kmm_utils_platformtools_HEADER
  platformtools.h
)

add_library(kmm_utils_validators STATIC ${kmm_utils_validators_SRCS})
add_library(kmm_utils_webconnect STATIC ${kmm_utils_webconnect_SRCS})
add_library(kmm_utils_platformtools STATIC ${kmm_utils_platformtools_SRCS})

target_link_libraries( kmm_utils_validators
  PUBLIC
  Qt5::Core
  Qt5::Gui
)

target_link_libraries( kmm_utils_webconnect
  PUBLIC
  Qt5::Core
  Qt5::Network
)

target_link_libraries( kmm_utils_platformtools
  PUBLIC
  Qt5::Core
)
