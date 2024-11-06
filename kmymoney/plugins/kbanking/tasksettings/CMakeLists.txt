set ( kbanking_tasksettings_SRCS
  credittransfersettingsbase.cpp
)

add_library( kbanking_tasksettings STATIC ${kbanking_tasksettings_SRCS} )

target_link_libraries( kbanking_tasksettings
  kmm_utils_validators
  Qt::Gui
  Qt::Xml
  KF${QT_MAJOR_VERSION}::I18n
  konlinetasks_sepa_interface
)
