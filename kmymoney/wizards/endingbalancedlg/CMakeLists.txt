INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET (libendingbalancedlg_a_SOURCES
  kendingbalancedlg.cpp
  checkingstartwizardpage.cpp
  checkingstatementinfowizardpage.cpp
  interestchargecheckingswizardpage.cpp
  previouspostponewizardpage.cpp
  )

SET (libendingbalancedlg_a_UI
  kendingbalancedlgdecl.ui
  checkingstartwizardpagedecl.ui
  checkingstatementinfowizardpagedecl.ui
  interestchargecheckingswizardpagedecl.ui
  previouspostponewizardpagedecl.ui
	)

kde4_add_ui_files(libendingbalancedlg_a_SOURCES ${libendingbalancedlg_a_UI} )

KDE4_ADD_LIBRARY(endingbalancedlg STATIC ${libendingbalancedlg_a_SOURCES})

# we rely on some widgets to be generated
ADD_DEPENDENCIES(endingbalancedlg widgets dialogs)
