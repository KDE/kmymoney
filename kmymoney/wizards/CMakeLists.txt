include_directories( ${CMAKE_CURRENT_BINARY_DIR}/wizardpages)

add_subdirectory( wizardpages )
add_subdirectory( newuserwizard )
add_subdirectory( newaccountwizard )
add_subdirectory( newinvestmentwizard )
add_subdirectory( newloanwizard )
add_subdirectory( endingbalancedlg )

set(kmm_wizard_SOURCES
  kmymoneywizard.cpp
  kmymoneywizardpage.cpp
)

add_library(kmm_wizard SHARED ${kmm_wizard_SOURCES})

generate_export_header(kmm_wizard BASE_NAME kmm_wizard)

target_link_libraries(kmm_wizard
    KF5::XmlGui
    KF5::TextWidgets
    kmm_icons
    kmm_base_widgets
    )

set_target_properties(kmm_wizard PROPERTIES
  VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
)

install(TARGETS kmm_wizard ${INSTALL_TARGETS_DEFAULT_ARGS} )
