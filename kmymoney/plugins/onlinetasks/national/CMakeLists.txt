# Todo move this to a findKtoblzcheck.cmake and use find_package() instead
find_library( KTOBLZCHECK_LIBRARY NAMES libktoblzcheck ktoblzcheck
    DOC "German account number and bank code check http://ktoblzcheck.sourceforge.net/")
message(STATUS "Found ktoblzcheck at " ${KTOBLZCHECK_LIBRARY})

set( konlinetasks_national_SCRS
  tasks/germanonlinetransferimpl.cpp
  ui/germancredittransferedit.cpp
  converter/taskconvertergermantosepa.cpp
  converter/taskconvertersepatogerman.cpp
  )

set( konlinetasks_national_HEADER
  tasks/germanonlinetransfer.h
)

kde4_add_ui_files( konlinetasks_national_SCRS
  ui/germancredittransferedit.ui
)

# Make this plugin unit testable
automoc4(konlinetasks_national_OBJECTS konlinetasks_national_SCRS)
add_library( konlinetasks_national_OBJECTS OBJECT ${konlinetasks_national_SCRS} )

# Add plugin which is not allowed in the unit test
kde4_add_plugin( konlinetasks_national
  $<TARGET_OBJECTS:konlinetasks_national_OBJECTS>
  nationalonlinetasksloader.cpp
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmymoney-nationalorders.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-nationalorders.desktop)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmymoney-nationalordersui.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-nationalordersui.desktop)

target_link_libraries( konlinetasks_national
  kmm_mymoney
  kmm_widgets
  kmymoney_base
  kmm_config
  kmm_plugin
  kmm_utils_validators
  onlinetask_interfaces
  payeeidentifier_nationalAccount
  ${QT_QTCORE_LIBRARIES}
  ${QT_QTGUI_LIBRARIES}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  ${QT_QTXML_LIBRARY}
  ${LIBALKIMIA_LIBRARY}
  ${GMP_LIBRARIES} # TODO: remove when alkimia cmake config file got fixed
  ${KTOBLZCHECK_LIBRARY}
)

install(TARGETS konlinetasks_national
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES
          ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-nationalorders.desktop
          ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-nationalordersui.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES ${konlinetasks_national_HEADER}
    DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney/onlinetasks/national/ COMPONENT Devel)

if( KDE4_BUILD_TESTS )
  add_subdirectory(tests)
endif()
