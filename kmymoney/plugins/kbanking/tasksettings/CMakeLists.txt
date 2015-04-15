set ( kbanking_tasksettings_SRCS
  credittransfersettingsbase.cpp
)

KDE4_ADD_LIBRARY( kmm_kbanking_tasksettings STATIC ${kbanking_tasksettings_SRCS} )

target_link_libraries( kmm_kbanking_tasksettings
  kmm_utils_validators
)
