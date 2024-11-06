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
target_link_libraries(endingbalancedlg
        KF${QT_MAJOR_VERSION}::ConfigWidgets
        KF${QT_MAJOR_VERSION}::WidgetsAddons
        KF${QT_MAJOR_VERSION}::I18n
        KF${QT_MAJOR_VERSION}::Completion
        Qt::Widgets
        Qt::Xml
        Alkimia::alkimia
        kmm_base_widgets
    )

# we rely on some widgets to be generated
# add_dependencies(endingbalancedlg widgets)
