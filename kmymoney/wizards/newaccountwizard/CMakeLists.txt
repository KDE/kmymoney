INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET (libnewaccountwizard_a_SOURCES 
  knewaccountwizard.cpp 
  )

SET (libnewaccountwizard_a_UI
  kaccountsummarypagedecl.ui kaccounttypepagedecl.ui
  kbrokeragepagedecl.ui kgeneralloaninfopagedecl.ui
  khierarchypagedecl.ui kinstitutionpagedecl.ui
  kloandetailspagedecl.ui kloanpaymentpagedecl.ui
  kloanpayoutpagedecl.ui kloanschedulepagedecl.ui kschedulepagedecl.ui
	)

kde4_add_ui_files(libnewaccountwizard_a_SOURCES ${libnewaccountwizard_a_UI} )

KDE4_ADD_LIBRARY(newaccountwizard STATIC ${libnewaccountwizard_a_SOURCES})

# we rely on some dialogs to be generated
ADD_DEPENDENCIES(newaccountwizard dialogs)
