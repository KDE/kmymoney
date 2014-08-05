project(Wizards)

include_directories( ${CMAKE_CURRENT_BINARY_DIR}/wizardpages)

add_subdirectory( wizardpages )
add_subdirectory( newuserwizard )
add_subdirectory( newaccountwizard )
add_subdirectory( newinvestmentwizard )
add_subdirectory( newloanwizard )
add_subdirectory( endingbalancedlg )
