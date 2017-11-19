include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

set (libnewuserwizard_a_SOURCES
  knewuserwizard.cpp
  kaccountpage.cpp
  kcategoriespage.cpp
  kcurrencypage.cpp
  kfilepage.cpp
  kgeneralpage.cpp
  kintropage.cpp
  kpreferencepage.cpp
  )

set (libnewuserwizard_a_UI
  kaccountpage.ui kcurrencypage.ui
  kfilepage.ui kgeneralpage.ui
  kintropage.ui kpreferencepage.ui
  kpasswordpage.ui
  )

# The handling of these ui files depends
# on libkmymoney.so (the widgets library)
ki18n_wrap_ui(libnewuserwizard_a_SOURCES ${libnewuserwizard_a_UI})

add_library(newuserwizard STATIC ${libnewuserwizard_a_SOURCES})
# TODO: clean dependencies
target_link_libraries(newuserwizard kmymoneywizard KF5::ConfigGui KF5::KIOWidgets KF5::TextWidgets KF5::Completion KF5::ConfigWidgets Qt5::Widgets Qt5::Xml Alkimia::alkimia)
add_dependencies(newuserwizard widgets wizardpages)

