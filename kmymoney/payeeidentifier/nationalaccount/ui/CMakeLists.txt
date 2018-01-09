configure_file(kmymoney-nationalaccount-delegate.desktop.in kmymoney-nationalaccount-delegate.desktop)

set( PAYEEIDENTIFIER_NATIONALACCOUNT_UI_SCRS
  nationalaccountedit.cpp
  nationalaccountdelegate.cpp
  pluginfactory.cpp
)

ki18n_wrap_ui(
  PAYEEIDENTIFIER_NATIONALACCOUNT_UI_SCRS
  nationalaccountedit.ui
)

add_library( payeeidentifier_nationalAccount_ui MODULE
  ${PAYEEIDENTIFIER_NATIONALACCOUNT_UI_SCRS}
)

target_link_libraries( payeeidentifier_nationalAccount_ui
  payeeidentifier_nationalAccount
  Qt5::Core
  kmm_widgets
)

install(TARGETS payeeidentifier_nationalAccount_ui
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-nationalaccount-delegate.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
