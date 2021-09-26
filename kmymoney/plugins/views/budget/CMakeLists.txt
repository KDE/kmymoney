configure_file(${CMAKE_CURRENT_SOURCE_DIR}/budgetview.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/budgetview.json @ONLY)

set(budgetview_SOURCES
  budgetview.cpp
  kbudgetview.cpp
  kbudgetvalues.cpp
  knewbudgetdlg.cpp
  )

ki18n_wrap_ui(budgetview_SOURCES
              kbudgetview.ui
              kbudgetvalues.ui
              knewbudgetdlg.ui
              )

qt5_add_resources(budgetview_SOURCES budgetview.qrc)

kmymoney_add_plugin(budgetview
                       SOURCES ${budgetview_SOURCES})

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(budgetview
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

target_link_libraries(budgetview
  PUBLIC
    kmm_plugin
    kmm_widgets
    kmymoney_common
)
