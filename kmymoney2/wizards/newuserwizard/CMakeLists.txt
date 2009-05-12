INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET (libnewuserwizard_a_SOURCES
  knewuserwizard.cpp
  )

SET (libnewuserwizard_a_UI
  kaccountpagedecl.ui kcurrencypagedecl.ui kfilepagedecl.ui
  kgeneralpagedecl.ui kintropagedecl.ui kpasswordpagedecl.ui
  kpreferencepagedecl.ui
  )

# The handling of these ui files depends
# on libkmymoney.so (the widgets library)
KMM_ADD_UI_FILES(libnewuserwizard_a_SOURCES ${libnewuserwizard_a_UI}
  DEPENDS kmymoney)

KDE4_ADD_LIBRARY(newuserwizard STATIC ${libnewuserwizard_a_SOURCES})
ADD_DEPENDENCIES(newuserwizard generate_wizardpages_ui_srcs)

