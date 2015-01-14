
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmymoneynationalaccountnumberplugin.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/kmymoneynationalaccountnumberplugin.desktop)

set (nationalAccountId_SCRS
  nationalaccount.cpp
)

kde4_add_library( payeeidentifier_nationalAccount
  SHARED
  ${nationalAccountId_SCRS}
)

target_link_libraries( payeeidentifier_nationalAccount
  kmm_payeeidentifier
  Qt5::Core
  Qt5::Xml
)

install(TARGETS payeeidentifier_nationalAccount
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmymoneynationalaccountnumberplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

#if( KDE4_BUILD_TESTS )
#  add_subdirectory(tests)
#endif()

add_subdirectory( ui )
