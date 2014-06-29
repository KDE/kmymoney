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
qt5_wrap_ui(libnewuserwizard_a_SOURCES ${libnewuserwizard_a_UI})

add_library(newuserwizard STATIC ${libnewuserwizard_a_SOURCES})
ADD_DEPENDENCIES(newuserwizard widgets wizardpages)

