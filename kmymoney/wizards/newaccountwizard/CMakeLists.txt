include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

set (libnewaccountwizard_a_SOURCES
  knewaccountwizard.cpp
  )

set (libnewaccountwizard_a_UI
  kaccountsummarypagedecl.ui kaccounttypepagedecl.ui
  kbrokeragepagedecl.ui kgeneralloaninfopagedecl.ui
  khierarchypagedecl.ui kinstitutionpagedecl.ui
  kloandetailspagedecl.ui kloanpaymentpagedecl.ui
  kloanpayoutpagedecl.ui kloanschedulepagedecl.ui kschedulepagedecl.ui
  )

qt5_wrap_ui(libnewaccountwizard_a_SOURCES ${libnewaccountwizard_a_UI} )

add_library(newaccountwizard STATIC ${libnewaccountwizard_a_SOURCES})
# TODO: clean dependencies
target_link_libraries(newaccountwizard KF5::XmlGui KF5::Service KF5::IconThemes KF5::ConfigGui KF5::WidgetsAddons KF5::TextWidgets KF5::Completion Qt5::Widgets Qt5::Xml Qt5::Sql Alkimia::alkimia)

# we rely on some dialogs to be generated
add_dependencies(newaccountwizard dialogs)
