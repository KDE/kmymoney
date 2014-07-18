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

qt5_wrap_ui(libnewaccountwizard_a_SOURCES ${libnewaccountwizard_a_UI} )

add_library(newaccountwizard STATIC ${libnewaccountwizard_a_SOURCES})
target_link_libraries(newaccountwizard Alkimia::alkimia)

# we rely on some dialogs to be generated
ADD_DEPENDENCIES(newaccountwizard dialogs)
