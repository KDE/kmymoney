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
kde4_add_ui_files(libnewuserwizard_a_SOURCES ${libnewuserwizard_a_UI})

kde4_add_library(newuserwizard STATIC ${libnewuserwizard_a_SOURCES})
add_dependencies(newuserwizard widgets wizardpages)

