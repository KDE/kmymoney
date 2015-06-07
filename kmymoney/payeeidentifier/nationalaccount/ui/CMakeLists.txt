configure_file(kmymoney-nationalaccount-delegate.desktop.in kmymoney-nationalaccount-delegate.desktop)

set ( PAYEEIDENTIFIER_NATIONALACCOUNT_UI_SCRS
  nationalaccountedit.cpp
  nationalaccountdelegate.cpp
  pluginfactory.cpp
)

kde4_add_ui_files(
  PAYEEIDENTIFIER_NATIONALACCOUNT_UI_SCRS
  nationalaccountedit.ui
)

kde4_add_plugin( payeeidentifier_nationalAccount_ui
  ${PAYEEIDENTIFIER_NATIONALACCOUNT_UI_SCRS}
)

target_link_libraries( payeeidentifier_nationalAccount_ui
  payeeidentifier_nationalAccount
  ${QT_QTCORE_LIBRARY}
  kmm_widgets
)

install(TARGETS payeeidentifier_nationalAccount_ui
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-nationalaccount-delegate.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
