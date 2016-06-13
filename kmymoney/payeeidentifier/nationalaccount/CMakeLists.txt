

set (nationalAccountId_SCRS
  nationalaccount.cpp
)

add_library( payeeidentifier_nationalAccount SHARED
  ${nationalAccountId_SCRS}
)
generate_export_header(payeeidentifier_nationalAccount BASE_NAME nationalaccount_identifier)

target_link_libraries( payeeidentifier_nationalAccount
  PUBLIC
    kmm_payeeidentifier
    Qt5::Core
    Qt5::Xml
    Qt5::Sql
)

set_target_properties(payeeidentifier_nationalAccount PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmymoney-nationalaccountnumberplugin.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-nationalaccountnumberplugin.desktop)
kcoreaddons_desktop_to_json(payeeidentifier_nationalAccount ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-nationalaccountnumberplugin.desktop)

install(TARGETS payeeidentifier_nationalAccount
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-nationalaccountnumberplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

# Storage plugin

configure_file(kmymoney-nationalaccount-storageplugin.desktop.in kmymoney-nationalaccount-storageplugin.desktop)

set ( nationalAccount_STORAGE_PLUGIN_SCRS
  nationalaccountstorageplugin.cpp
)

kde4_add_plugin( payeeidentifier_nationalaccount_storageplugin
  ${nationalAccount_STORAGE_PLUGIN_SCRS}
)

target_link_libraries( payeeidentifier_nationalaccount_storageplugin
  ${QT_QTCORE_LIBRARY}
  ${QT_QTSQL_LIBRARY}
  kmm_mymoney
)

install(TARGETS payeeidentifier_nationalaccount_storageplugin
  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kmymoney-nationalaccount-storageplugin.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
  )

#if( KDE4_BUILD_TESTS )
#  add_subdirectory(tests)
#endif()

add_subdirectory( ui )
