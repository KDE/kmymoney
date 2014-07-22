include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

set (libendingbalancedlg_a_SOURCES
  kendingbalancedlg.cpp
  checkingstartwizardpage.cpp
  checkingstatementinfowizardpage.cpp
  interestchargecheckingswizardpage.cpp
  previouspostponewizardpage.cpp
  )

set (libendingbalancedlg_a_UI
  kendingbalancedlgdecl.ui
  checkingstartwizardpagedecl.ui
  checkingstatementinfowizardpagedecl.ui
  interestchargecheckingswizardpagedecl.ui
  previouspostponewizardpagedecl.ui
  )

kde4_add_ui_files(libendingbalancedlg_a_SOURCES ${libendingbalancedlg_a_UI} )

kde4_add_library(endingbalancedlg STATIC ${libendingbalancedlg_a_SOURCES})

# we rely on some widgets to be generated
add_dependencies(endingbalancedlg widgets dialogs)
