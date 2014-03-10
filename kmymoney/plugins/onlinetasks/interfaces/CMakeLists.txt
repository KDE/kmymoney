
set ( onlinetask_interfaces_SRCS
  converter/onlinetaskconverter.cpp
  tasks/onlinetask.cpp
  ui/ionlinejobedit.cpp
)

kde4_add_library(onlinetask_interfaces STATIC ${onlinetask_interfaces_SRCS} )

target_link_libraries( onlinetask_interfaces
  ${QT_QTCORE_LIBRARIES}
  ${QT_QTGUI_LIBRARIES}
)