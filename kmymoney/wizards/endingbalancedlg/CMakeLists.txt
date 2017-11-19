include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

set (libendingbalancedlg_a_SOURCES
  kendingbalancedlg.cpp
  checkingstartwizardpage.cpp
  checkingstatementinfowizardpage.cpp
  interestchargecheckingswizardpage.cpp
  previouspostponewizardpage.cpp
  )

set (libendingbalancedlg_a_UI
  kendingbalancedlg.ui
  checkingstartwizardpage.ui
  checkingstatementinfowizardpage.ui
  interestchargecheckingswizardpage.ui
  previouspostponewizardpage.ui
  )

ki18n_wrap_ui(libendingbalancedlg_a_SOURCES ${libendingbalancedlg_a_UI} )

add_library(endingbalancedlg STATIC ${libendingbalancedlg_a_SOURCES})
target_link_libraries(endingbalancedlg KF5::ConfigWidgets KF5::WidgetsAddons KF5::I18n KF5::Completion Qt5::Widgets Qt5::Sql Qt5::Xml Alkimia::alkimia)

# we rely on some widgets to be generated
add_dependencies(endingbalancedlg widgets dialogs)
