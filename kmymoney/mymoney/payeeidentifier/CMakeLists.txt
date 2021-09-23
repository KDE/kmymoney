set ( PAYEEIDENTIFIER_HEADER
  payeeidentifier.h
  payeeidentifiertyped.h
  payeeidentifierdata.h
)

set ( PAYEEIDENTIFIER_SCRS
  payeeidentifier.cpp
)

add_library( kmm_payeeidentifier
  ${PAYEEIDENTIFIER_SCRS}
)

set_target_properties(kmm_payeeidentifier PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

generate_export_header( kmm_payeeidentifier )

target_link_libraries( kmm_payeeidentifier
  Qt5::Core
  Qt5::Xml
)

target_include_directories( kmm_payeeidentifier PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
)

install( TARGETS kmm_payeeidentifier
  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
