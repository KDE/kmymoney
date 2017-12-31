set( sepaOnlineTasks_SRCS
  tasks/sepaonlinetransferimpl.cpp
  ui/sepacredittransferedit.cpp
  sepastorageplugin.cpp
  sepaonlinetasksloader.cpp
)

set( sepaOnlineTasks_HEADER
  tasks/sepaonlinetransfer.h
)

kde4_add_ui_files( sepaOnlineTasks_SRCS
  ui/sepacredittransferedit.ui
)

add_library(konlinetasks_sepa MODULE
  ${sepaOnlineTasks_SRCS}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmymoney-sepaorders.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-sepaorders.desktop)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmymoney-sepaordersui.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-sepaordersui.desktop)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmymoney-sepastorageplugin.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-sepastorageplugin.desktop)

target_link_libraries( konlinetasks_sepa
  kmm_mymoney
  kmm_widgets
  kmm_config
  kmm_plugin
  kmm_utils_validators
  onlinetask_interfaces
  payeeidentifier_nationalAccount
  payeeidentifier_iban_bic_widgets
  ${QT_QTCORE_LIBRARIES}
  ${QT_QTGUI_LIBRARIES}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  ${QT_QTXML_LIBRARY}
  ${LIBALKIMIA_LIBRARY}
  ${GMP_LIBRARIES} # TODO: remove when alkimia cmake config file got fixed
)

install(TARGETS konlinetasks_sepa
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES
          ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-sepaorders.desktop
          ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-sepaordersui.desktop
          ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-sepastorageplugin.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES ${sepaOnlineTasks_HEADER}
        DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney/onlinetasks/sepa/ COMPONENT Devel)
