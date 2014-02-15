# Todo move this to a findKtoblzcheck.cmake and use find_package() instead
find_library( KTOBLZCHECK_LIBRARY NAMES libktoblzcheck ktoblzcheck
    DOC "German account number and bank code check http://ktoblzcheck.sourceforge.net/")
message(STATUS "Found ktoblzcheck at " ${KTOBLZCHECK_LIBRARY})

set( sepaOnlineTasks_SRCS
  credittransfersettingsbase.cpp
  tasks/sepaonlinetransfer.cpp
  ui/sepacredittransferedit.cpp
)

kde4_add_ui_files( sepaOnlineTasks_SRCS
  ui/sepacredittransferedit.ui
)

kde4_add_library(konlinetasks_sepa STATIC ${sepaOnlineTasks_SRCS} )

# TODO: create plugin
#KDE4_ADD_PLUGIN(konlinetaskssepa
#  ${sepaOnlineTasks_SRCS}
#  )

target_link_libraries( konlinetasks_sepa
  onlinetask_interfaces
  ${QT_QTCORE_LIBRARIES}
  ${QT_QTGUI_LIBRARIES}
  ${QT_QTXML_LIBRARY}
  ${KTOBLZCHECK_LIBRARY}
)