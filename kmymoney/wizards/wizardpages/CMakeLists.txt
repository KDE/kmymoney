project(Wizardpages)

########### next target ###############
set(libwizardpages_a_SOURCES
  accounts.cpp currency.cpp userinfo.cpp
  )

set (libwizardpages_a_UI
  accountsdecl.ui currencydecl.ui userinfodecl.ui
  )

qt5_wrap_ui(wizardpages_ui_srcs ${libwizardpages_a_UI} )
# in order to use add_dependencies, we need to add this custom target
# for all generated header files.
add_custom_target(generate_wizardpages_ui_srcs DEPENDS
  ${wizardpages_ui_srcs})

add_library(wizardpages STATIC
  ${libwizardpages_a_SOURCES} ${wizardpages_ui_srcs})
target_link_libraries(wizardpages KF5::KDELibs4Support Alkimia::alkimia)

