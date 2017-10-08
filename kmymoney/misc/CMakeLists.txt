install(PROGRAMS financequote.pl
        DESTINATION ${DATA_INSTALL_DIR}/kmymoney/misc)

set( kmm_utils_validators_SRCS
  charvalidator.cpp
  validators.cpp
)

set( kmm_utils_webconnect_SRCS
  webconnect.cpp
)

set( kmm_utils_validators_HEADER
  charvalidator.h
  validators.h
)

set( kmm_utils_webconnect_HEADER
  webconnect.h
)

add_library(kmm_utils_validators STATIC ${kmm_utils_validators_SRCS})
add_library(kmm_utils_webconnect STATIC ${kmm_utils_webconnect_SRCS})

target_link_libraries( kmm_utils_validators
  PUBLIC
  Qt5::Core
)

target_link_libraries( kmm_utils_webconnect
  PUBLIC
  Qt5::Core
)
