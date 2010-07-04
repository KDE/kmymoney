INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET (libnewloanwizard_a_SOURCES
  knewloanwizard.cpp
  keditloanwizard.cpp
  )

SET (libnewloanwizard_a_UI
  knewloanwizarddecl.ui
	)

kde4_add_ui_files(libnewloanwizard_a_SOURCES ${libnewloanwizard_a_UI} )

KDE4_ADD_LIBRARY(newloanwizard STATIC ${libnewloanwizard_a_SOURCES})

# we rely on some widgets to be generated
#ADD_DEPENDENCIES(newinvestmentwizard widgets)
