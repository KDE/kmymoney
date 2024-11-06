include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

set (libnewaccountwizard_a_SOURCES
  knewaccountwizard.cpp
  kaccountsummarypage.cpp
  kaccounttypepage.cpp
  kbrokeragepage.cpp
  kcreditcardschedulepage.cpp
  kgeneralloaninfopage.cpp
  khierarchypage.cpp
  kinstitutionpage.cpp
  kloandetailspage.cpp
  kloanpaymentpage.cpp
  kloanpayoutpage.cpp
  kloanschedulepage.cpp
  )

set (libnewaccountwizard_a_UI
  kaccountsummarypage.ui kaccounttypepage.ui
  kbrokeragepage.ui kgeneralloaninfopage.ui
  kgeneralloaninfopage.ui kinstitutionpage.ui
  kloandetailspage.ui kloanpaymentpage.ui
  kloanpayoutpage.ui kloanschedulepage.ui kcreditcardschedulepage.ui khierarchypage.ui
  )

ki18n_wrap_ui(libnewaccountwizard_a_SOURCES ${libnewaccountwizard_a_UI} )

add_library(newaccountwizard STATIC ${libnewaccountwizard_a_SOURCES})
# TODO: clean dependencies
target_link_libraries(newaccountwizard
  KF${QT_MAJOR_VERSION}::XmlGui
  KF${QT_MAJOR_VERSION}::Service
  KF${QT_MAJOR_VERSION}::ConfigGui
  KF${QT_MAJOR_VERSION}::WidgetsAddons
  KF${QT_MAJOR_VERSION}::TextWidgets
  KF${QT_MAJOR_VERSION}::Completion
  KF${QT_MAJOR_VERSION}::ItemModels
  Qt::Widgets
  Qt::Xml
  Alkimia::alkimia
  kmm_wizard
  )

# we rely on some dialogs to be generated
# add_dependencies(newaccountwizard)
