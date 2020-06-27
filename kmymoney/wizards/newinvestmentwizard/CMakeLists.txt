include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

set (libnewinvestmentwizard_a_SOURCES
  knewinvestmentwizard.cpp
  konlineupdatewizardpage.cpp
  kinvestmentdetailswizardpage.cpp
  kinvestmenttypewizardpage.cpp
  )

set (libnewinvestmentwizard_a_UI
  knewinvestmentwizard.ui
  kinvestmentdetailswizardpage.ui
  kinvestmenttypewizardpage.ui
  konlineupdatewizardpage.ui
  )

ki18n_wrap_ui(libnewinvestmentwizard_a_SOURCES ${libnewinvestmentwizard_a_UI} )

add_library(newinvestmentwizard STATIC ${libnewinvestmentwizard_a_SOURCES})
# TODO: cleanup dependencies
target_link_libraries(newinvestmentwizard KF5::I18n KF5::ConfigWidgets KF5::Completion KF5::KIOWidgets KF5::WidgetsAddons Alkimia::alkimia Qt5::Widgets Qt5::Xml kmm_webconnect)

# we rely on some widgets to be generated
# add_dependencies(newinvestmentwizard widgets)
