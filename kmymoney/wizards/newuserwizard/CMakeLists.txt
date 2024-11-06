include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

set (libnewuserwizard_a_SOURCES
  knewuserwizard.cpp
  kaccountpage.cpp
  kcategoriespage.cpp
  kcurrencypage.cpp
  kgeneralpage.cpp
  kintropage.cpp
  kpreferencepage.cpp
  )

set (libnewuserwizard_a_UI
  kaccountpage.ui kcurrencypage.ui
  kgeneralpage.ui
  kintropage.ui kpreferencepage.ui
  kpasswordpage.ui
  )

# The handling of these ui files depends
# on libkmymoney.so (the widgets library)
ki18n_wrap_ui(libnewuserwizard_a_SOURCES ${libnewuserwizard_a_UI})

add_library(newuserwizard STATIC ${libnewuserwizard_a_SOURCES})


# TODO: clean dependencies
target_link_libraries(newuserwizard
    KF${QT_MAJOR_VERSION}::ConfigGui
    KF${QT_MAJOR_VERSION}::KIOWidgets
    KF${QT_MAJOR_VERSION}::TextWidgets
    KF${QT_MAJOR_VERSION}::Completion
    KF${QT_MAJOR_VERSION}::ConfigWidgets
    Qt::Widgets
    Qt::Xml
    Alkimia::alkimia
    kmm_mymoney
    kmm_wizard
    kmm_templates
    kmm_widgets
)

add_dependencies(newuserwizard wizardpages)

