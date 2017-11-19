project(Wizards)

include_directories( ${CMAKE_CURRENT_BINARY_DIR}/wizardpages)

add_subdirectory( wizardpages )
add_subdirectory( newuserwizard )
add_subdirectory( newaccountwizard )
add_subdirectory( newinvestmentwizard )
add_subdirectory( newloanwizard )
add_subdirectory( endingbalancedlg )

set(kmymoneywizard_sources
  kmymoneywizard.cpp
  kmymoneywizardpage.cpp
  )

add_library(kmymoneywizard STATIC ${kmymoneywizard_sources})

target_link_libraries(kmymoneywizard KF5::XmlGui KF5::TextWidgets)
