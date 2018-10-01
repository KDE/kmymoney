include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

set (libnewinvestmentwizard_a_SOURCES
  knewinvestmentwizard.cpp
  konlineupdatewizardpage.cpp
  kinvestmentdetailswizardpage.cpp
  kinvestmenttypewizardpage.cpp
  )

set (libnewinvestmentwizard_a_UI
  knewinvestmentwizarddecl.ui
  kinvestmentdetailswizardpagedecl.ui
  kinvestmenttypewizardpagedecl.ui
  konlineupdatewizardpagedecl.ui
  )

kde4_add_ui_files(libnewinvestmentwizard_a_SOURCES ${libnewinvestmentwizard_a_UI} )

kde4_add_library(newinvestmentwizard STATIC ${libnewinvestmentwizard_a_SOURCES})
target_link_libraries(newinvestmentwizard ${LIBALKIMIA_LIBRARIES})

# we rely on some widgets to be generated
add_dependencies(newinvestmentwizard widgets)
