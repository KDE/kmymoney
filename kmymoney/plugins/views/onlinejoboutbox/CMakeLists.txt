configure_file(${CMAKE_CURRENT_SOURCE_DIR}/onlinejoboutboxview.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/onlinejoboutboxview.json @ONLY)

set(onlinejoboutboxview_SOURCES
  onlinejoboutboxview.cpp
  konlinejoboutboxview.cpp
  konlinetransferform.cpp
  onlinejobmodel.cpp
  onlinejobmessagesmodel.cpp
  )

ki18n_wrap_ui(onlinejoboutboxview_SOURCES
  konlinejoboutboxview.ui
  konlinetransferform.ui
  )

qt5_add_resources(onlinejoboutboxview_SOURCES onlinejoboutboxview.qrc)

kmymoney_add_plugin(onlinejoboutboxview SOURCES ${onlinejoboutboxview_SOURCES})

target_link_libraries(onlinejoboutboxview
  PUBLIC
    kmm_plugin
    kmm_models
    kmm_widgets
    kmm_base_widgets
    onlinetask_interfaces
    KF5::TextWidgets
    kmm_menus
)
