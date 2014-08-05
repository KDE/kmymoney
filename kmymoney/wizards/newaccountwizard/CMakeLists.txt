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

kde4_add_ui_files(libnewaccountwizard_a_SOURCES ${libnewaccountwizard_a_UI} )

kde4_add_library(newaccountwizard STATIC ${libnewaccountwizard_a_SOURCES})

# we rely on some dialogs to be generated
add_dependencies(newaccountwizard dialogs)
