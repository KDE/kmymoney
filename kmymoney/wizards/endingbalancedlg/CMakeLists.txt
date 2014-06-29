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

qt5_wrap_ui(libendingbalancedlg_a_SOURCES ${libendingbalancedlg_a_UI} )

add_library(endingbalancedlg STATIC ${libendingbalancedlg_a_SOURCES})

# we rely on some widgets to be generated
ADD_DEPENDENCIES(endingbalancedlg widgets dialogs)
