INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET (libnewinvestmentwizard_a_SOURCES
  knewinvestmentwizard.cpp
  konlineupdatewizardpage.cpp
  kinvestmentdetailswizardpage.cpp
  kinvestmenttypewizardpage.cpp
  )

SET (libnewinvestmentwizard_a_UI
  knewinvestmentwizarddecl.ui
  kinvestmentdetailswizardpagedecl.ui
  kinvestmenttypewizardpagedecl.ui
  konlineupdatewizardpagedecl.ui
	)

kde4_add_ui_files(libnewinvestmentwizard_a_SOURCES ${libnewinvestmentwizard_a_UI} )

KDE4_ADD_LIBRARY(newinvestmentwizard STATIC ${libnewinvestmentwizard_a_SOURCES})

# we rely on some widgets to be generated
ADD_DEPENDENCIES(newinvestmentwizard widgets)
