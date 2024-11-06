
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmymoney-sepaorders.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-sepaorders.json @ONLY)

set( sepaOnlineTasks_SRCS
  ui/sepacredittransferedit.cpp
  sepaonlinetasksloader.cpp
)

ki18n_wrap_ui( sepaOnlineTasks_SRCS
  ui/sepacredittransferedit.ui
)

kmymoney_add_plugin(konlinetasks_sepa SOURCES ${sepaOnlineTasks_SRCS})

target_link_libraries(konlinetasks_sepa
  PUBLIC
    konlinetasks_sepa_interface
  PRIVATE
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::Service
    kmm_settings
    kmm_utils_validators
    KF${QT_MAJOR_VERSION}::ItemModels
)

add_library(konlinetasks_sepa_interface INTERFACE)

# TODO some dependencies can be moved to konlinetasks_sepa
target_link_libraries( konlinetasks_sepa_interface
  INTERFACE
  kmm_mymoney
  kmm_base_widgets
  kmm_plugin
  onlinetask_interfaces
  Qt::Xml
  Qt::Core
  Qt::Gui
)
