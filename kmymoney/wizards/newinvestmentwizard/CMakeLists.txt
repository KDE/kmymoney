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
target_link_libraries(newinvestmentwizard
       KF${QT_MAJOR_VERSION}::I18n
       KF${QT_MAJOR_VERSION}::ConfigWidgets
       KF${QT_MAJOR_VERSION}::Completion
       KF${QT_MAJOR_VERSION}::KIOWidgets
       KF${QT_MAJOR_VERSION}::WidgetsAddons
       Alkimia::alkimia
       Qt::Widgets
       Qt::Xml
       kmm_webconnect
)
