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
    KF5::ConfigGui
    KF5::KIOWidgets
    KF5::TextWidgets
    KF5::Completion
    KF5::ConfigWidgets
    Qt::Widgets
    Qt::Xml
    Alkimia::alkimia
    kmm_mymoney
    kmm_wizard
    kmm_templates
    kmm_widgets
)

add_dependencies(newuserwizard wizardpages)

