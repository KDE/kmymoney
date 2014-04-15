
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmymoneynationalaccountnumberplugin.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/kmymoneynationalaccountnumberplugin.desktop)

set (nationalAccountId_SCRS
  nationalaccountid.cpp
)

kde4_add_library( payeeidentifier_nationalAccount
  SHARED
  ${nationalAccountId_SCRS}
)

target_link_libraries( payeeidentifier_nationalAccount
  ${QT_QTCORE_LIBRARY}
  ${QT_QTXML_LIBRARY}
)

install(TARGETS payeeidentifier_nationalAccount
    DESTINATION ${PLUGIN_INSTALL_DIR}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmymoneynationalaccountnumberplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

#if( KDE4_BUILD_TESTS )
#  add_subdirectory(tests)
#endif()
