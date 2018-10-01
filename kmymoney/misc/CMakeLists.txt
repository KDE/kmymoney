set( kmm_utils_validators_SRCS
  validators.cpp
  charvalidator.cpp
)

set( kmm_utils_validators_HEADER
  validators.h
  charvalidator.h
)

kde4_add_library(kmm_utils_validators STATIC ${kmm_utils_validators_SRCS})

target_link_libraries( kmm_utils_validators
  ${QT_QTCORE_LIBRARIES}
)

#INSTALL(TARGETS kmm_utils_validators
#    ${INSTALL_TARGETS_DEFAULT_ARGS} )

INSTALL(FILES ${kmm_utils_validators_HEADER}
    DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney/misc COMPONENT Devel)
