set ( PAYEEIDENTIFIER_HEADER
  payeeidentifier.h
  payeeidentifiertyped.h
  payeeidentifierdata.h
)

set ( PAYEEIDENTIFIER_SCRS
  payeeidentifier.cpp
)

add_library( kmm_payeeidentifier SHARED
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

## Plugin loader

set ( PAYEEIDENTIFIER_LOADER_SCRS
  payeeidentifierloader.cpp
)

add_library( kmm_payeeidentifier_loader SHARED
  ${PAYEEIDENTIFIER_LOADER_SCRS}
)

generate_export_header(kmm_payeeidentifier_loader BASE_NAME kmm_payeeidentifier_loader)

target_link_libraries( kmm_payeeidentifier_loader
  PUBLIC
    Qt5::Core
  PRIVATE
    Qt5::Widgets
    KF5::Service
)

install(TARGETS kmm_payeeidentifier_loader
  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
