# Todo move this to a findKtoblzcheck.cmake and use find_package() instead
find_library( KTOBLZCHECK_LIBRARY NAMES libktoblzcheck ktoblzcheck
    DOC "German account number and bank code check http://ktoblzcheck.sourceforge.net/")
message(STATUS "Found ktoblzcheck at " ${KTOBLZCHECK_LIBRARY})

set( konlinetasks_national_SCRS
  tasks/germanonlinetransfer.cpp
  ui/germancredittransferedit.cpp
  ../sepa/credittransfersettingsbase.cpp
  converter/taskconvertergermantosepa.cpp
  converter/taskconvertersepatogerman.cpp
  )
  
kde4_add_ui_files( konlinetasks_national_SCRS
  ui/germancredittransferedit.ui
)

kde4_add_library( konlinetasks_national STATIC
  ${konlinetasks_national_SCRS}
)

target_link_libraries( konlinetasks_national
  onlinetask_interfaces
  ${QT_AND_KDECORE_LIBS}
  ${QT_QTCORE_LIBRARIES}
  ${QT_QTGUI_LIBRARIES}
  ${QT_QTXML_LIBRARY}
  ${GMP_LIBRARIES}
)

IF( KDE4_BUILD_TESTS )
  add_subdirectory(tests)
ENDIF( KDE4_BUILD_TESTS )
