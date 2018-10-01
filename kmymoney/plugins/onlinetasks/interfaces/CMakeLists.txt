
set ( onlinetask_interfaces_HEADER
  converter/onlinetaskconverter.h
  tasks/onlinetask.h
  tasks/credittransfer.h
  tasks/onlinetasksettingsfactory.h
  tasks/ionlinetasksettings.h
  ui/ionlinejobedit.h
)

set ( onlinetask_interfaces_SRCS
  converter/onlinetaskconverter.cpp
  tasks/onlinetask.cpp
  ui/ionlinejobedit.cpp
)

# At the moment onlinetask_interfaces is linked into kmm_mymoney and should not be used anywhere else!
kde4_add_library(onlinetask_interfaces STATIC ${onlinetask_interfaces_SRCS})

target_link_libraries( onlinetask_interfaces
  kmm_payeeidentifier
  ${QT_QTCORE_LIBRARIES}
  ${ALKIMIA_LIBRARIES}
)

install(FILES ${onlinetask_interfaces_HEADER}
    DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney/onlinetasks/interfaces/ COMPONENT Devel)
