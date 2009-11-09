PROJECT(Wizards)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR}/wizardpages)

ADD_SUBDIRECTORY( wizardpages )
ADD_SUBDIRECTORY( newuserwizard ) 
ADD_SUBDIRECTORY( newaccountwizard )
