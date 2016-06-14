set( sepaOnlineTasks_SRCS
  tasks/sepaonlinetransferimpl.cpp
  ui/sepacredittransferedit.cpp
  sepastorageplugin.cpp
  sepaonlinetasksloader.cpp
)

set( sepaOnlineTasks_HEADER
  tasks/sepaonlinetransfer.h
)

qt5_wrap_ui( sepaOnlineTasks_SRCS
  ui/sepacredittransferedit.ui
)

add_library(konlinetasks_sepa_interface INTERFACE)

add_library(konlinetasks_sepa MODULE
  ${sepaOnlineTasks_SRCS}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmymoney-sepaorders.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-sepaorders.desktop)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmymoney-sepaordersui.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-sepaordersui.desktop)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmymoney-sepastorageplugin.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-sepastorageplugin.desktop)
kcoreaddons_desktop_to_json(konlinetasks_sepa ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-sepaorders.desktop ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-sepaordersui.desktop)

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
  Qt5::Sql
)

target_link_libraries(konlinetasks_sepa
  PUBLIC
    konlinetasks_sepa_interface
  PRIVATE
    KF5::I18n
    KF5::Service
    kmm_config
    kmm_utils_validators
    payeeidentifier_iban_bic_widgets
    KF5::KDELibs4Support
    KF5::ItemModels
    #KF5::KIO TODO: Really not needed?!
)

install(TARGETS konlinetasks_sepa
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES
          ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-sepaorders.desktop
          ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-sepaordersui.desktop
          ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-sepastorageplugin.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES ${sepaOnlineTasks_HEADER}
        DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney/onlinetasks/sepa/
        COMPONENT Devel)
