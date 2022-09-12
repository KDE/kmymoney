
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
add_library(onlinetask_interfaces ${onlinetask_interfaces_SRCS})

target_link_libraries( onlinetask_interfaces
  PUBLIC
    kmm_payeeidentifier
    Qt::Widgets
    Alkimia::alkimia
)

set_target_properties(onlinetask_interfaces PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

generate_export_header(onlinetask_interfaces BASE_NAME onlinetask_interfaces)

install(TARGETS onlinetask_interfaces ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES ${onlinetask_interfaces_HEADER}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kmymoney/onlinetasks/interfaces/ COMPONENT Devel)
