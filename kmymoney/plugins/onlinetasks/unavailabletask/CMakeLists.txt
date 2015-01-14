
set( UNAVAILABLE_TASK_SRCS
  tasks/unavailabletask.cpp
)

add_library( onlinetask_unavailabletask STATIC
  ${UNAVAILABLE_TASK_SRCS}
)

target_link_libraries( onlinetask_unavailabletask
  onlinetask_interfaces
  Qt5::Core
  Qt5::Xml
  KF5::I18n
)
