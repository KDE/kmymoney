configure_file(${CMAKE_CURRENT_SOURCE_DIR}/onlinejoboutboxview.json.in ${CMAKE_CURRENT_BINARY_DIR}/onlinejoboutboxview.json @ONLY)

set(onlinejoboutboxview_SOURCES
  onlinejoboutboxview.cpp
  konlinejoboutboxview.cpp
  konlinetransferform.cpp
  onlinejobmodel.cpp
  onlinejobmessagesmodel.cpp
  ${CMAKE_SOURCE_DIR}/kmymoney/widgets/kmymoneyaccountcombo.cpp
  )

ki18n_wrap_ui(onlinejoboutboxview_SOURCES
  konlinejoboutboxview.ui
  konlinetransferform.ui
  )

kcoreaddons_add_plugin(onlinejoboutboxview
                       SOURCES ${onlinejoboutboxview_SOURCES}
                       JSON "${CMAKE_CURRENT_BINARY_DIR}/onlinejoboutboxview.json"
                       INSTALL_NAMESPACE "kmymoney")

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(onlinejoboutboxview
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

target_link_libraries(onlinejoboutboxview
  PUBLIC
    kmm_plugin
    models
    onlinetask_interfaces
    KF5::TextWidgets
    viewbase
    kmm_menus
)
