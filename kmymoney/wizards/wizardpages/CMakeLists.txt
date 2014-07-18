PROJECT(Wizardpages)

########### next target ###############
SET(libwizardpages_a_SOURCES
  accounts.cpp currency.cpp userinfo.cpp
  )

SET (libwizardpages_a_UI
  accountsdecl.ui currencydecl.ui userinfodecl.ui
  )

qt5_wrap_ui(wizardpages_ui_srcs ${libwizardpages_a_UI} )
# in order to use add_dependencies, we need to add this custom target
# for all generated header files.
ADD_CUSTOM_TARGET(generate_wizardpages_ui_srcs DEPENDS
  ${wizardpages_ui_srcs})

add_library(wizardpages STATIC
  ${libwizardpages_a_SOURCES} ${wizardpages_ui_srcs})
target_link_libraries(wizardpages Alkimia::alkimia)

