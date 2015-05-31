
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmymoney-nationalaccountnumberplugin.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-nationalaccountnumberplugin.desktop)

set (nationalAccountId_SCRS
  nationalaccount.cpp
)

kde4_add_plugin( payeeidentifier_nationalAccount SHARED
  ${nationalAccountId_SCRS}
)
generate_export_header(payeeidentifier_nationalAccount BASE_NAME nationalaccount_identifier)

target_link_libraries( payeeidentifier_nationalAccount
  kmm_payeeidentifier
  ${QT_QTCORE_LIBRARY}
  ${QT_QTXML_LIBRARY}
)

install(TARGETS payeeidentifier_nationalAccount
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-nationalaccountnumberplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

#if( KDE4_BUILD_TESTS )
#  add_subdirectory(tests)
#endif()

add_subdirectory( ui )
