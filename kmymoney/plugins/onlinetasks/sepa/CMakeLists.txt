
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmymoney-sepaorders.json.in ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-sepaorders.json @ONLY)

set( sepaOnlineTasks_SRCS
  ui/sepacredittransferedit.cpp
  sepaonlinetasksloader.cpp
)

ki18n_wrap_ui( sepaOnlineTasks_SRCS
  ui/sepacredittransferedit.ui
)

add_library(konlinetasks_sepa_interface INTERFACE)

kcoreaddons_add_plugin(konlinetasks_sepa
                       SOURCES ${sepaOnlineTasks_SRCS}
                       JSON "${CMAKE_CURRENT_BINARY_DIR}/kmymoney-sepaorders.json"
                       INSTALL_NAMESPACE "kmymoney")

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(konlinetasks_sepa
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

# TODO some dependencies can be moved to konlinetasks_sepa
target_link_libraries( konlinetasks_sepa_interface
  INTERFACE
  kmm_mymoney
  kmm_widgets
  kmm_plugin
  onlinetask_interfaces
  Qt5::Xml
  Qt5::Core
  Qt5::Gui
)

target_link_libraries(konlinetasks_sepa
  PUBLIC
    konlinetasks_sepa_interface
    Alkimia::alkimia
  PRIVATE
    KF5::I18n
    KF5::Service
    kmm_settings
    kmm_utils_validators
    KF5::ItemModels
)

