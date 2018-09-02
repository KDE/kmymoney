
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
  tasks/credittransfer.cpp
  ui/ionlinejobedit.cpp
)

# At the moment onlinetask_interfaces is linked into kmm_mymoney and should not be used anywhere else!
add_library(onlinetask_interfaces STATIC ${onlinetask_interfaces_SRCS})

target_link_libraries( onlinetask_interfaces
  PUBLIC
    kmm_payeeidentifier
    Qt5::Widgets
    Alkimia::alkimia
)

install(FILES ${onlinetask_interfaces_HEADER}
    DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney/onlinetasks/interfaces/ COMPONENT Devel)
