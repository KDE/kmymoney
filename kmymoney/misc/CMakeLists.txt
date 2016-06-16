install(PROGRAMS financequote.pl
        DESTINATION ${DATA_INSTALL_DIR}/kmymoney/misc)


set( kmm_utils_validators_SRCS
  validators.cpp
  charvalidator.cpp
)

set( kmm_utils_validators_HEADER
  validators.h
  charvalidator.h
)

add_library(kmm_utils_validators STATIC ${kmm_utils_validators_SRCS})

target_link_libraries( kmm_utils_validators
  PUBLIC
  Qt5::Core
)
