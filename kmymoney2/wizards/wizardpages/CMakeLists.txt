PROJECT(Wizardpages)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

########### next target ###############
SET(libwizardpages_a_SOURCES
  accounts.cpp currency.cpp userinfo.cpp
  )

SET (libwizardpages_a_UI
  accountsdecl.ui currencydecl.ui userinfodecl.ui
  )

# The handling of these ui files depends
# on libkmymoney.so (the widgets library)
KMM_ADD_UI_FILES(wizardpages_ui_srcs ${libwizardpages_a_UI}
  DEPENDS kmymoney)
# in order to use add_dependencies, we need to add this custom target
# for all generated header files.
ADD_CUSTOM_TARGET(generate_wizardpages_ui_srcs DEPENDS
  ${wizardpages_ui_srcs})

KDE4_ADD_LIBRARY(wizardpages STATIC
  ${libwizardpages_a_SOURCES} ${wizardpages_ui_srcs})

