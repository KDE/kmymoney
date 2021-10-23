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

kmymoney_add_plugin(budgetview SOURCES ${budgetview_SOURCES})

target_link_libraries(budgetview
  PUBLIC
    kmm_plugin
    kmm_widgets
    kmymoney_common
)
