include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

set (libnewuserwizard_a_SOURCES
  knewuserwizard.cpp
  )

set (libnewuserwizard_a_UI
  kaccountpagedecl.ui kcurrencypagedecl.ui kfilepagedecl.ui
  kgeneralpagedecl.ui kintropagedecl.ui kpasswordpagedecl.ui
  kpreferencepagedecl.ui
  )

# The handling of these ui files depends
# on libkmymoney.so (the widgets library)
qt5_wrap_ui(libnewuserwizard_a_SOURCES ${libnewuserwizard_a_UI})

add_library(newuserwizard STATIC ${libnewuserwizard_a_SOURCES})
# TODO: clean dependencies
target_link_libraries(newuserwizard KF5::ConfigGui KF5::KIOWidgets KF5::TextWidgets KF5::Completion Qt5::Widgets Qt5::Sql Qt5::Xml Alkimia::alkimia)
add_dependencies(newuserwizard widgets wizardpages)

