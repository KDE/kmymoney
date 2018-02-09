add_subdirectory(payeeidentifier)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sqlstorage.json.in ${CMAKE_CURRENT_BINARY_DIR}/sqlstorage.json @ONLY)

set(sqlstorage_SOURCES
  sqlstorage.cpp
  mymoneystoragesql.cpp
  mymoneydbdef.cpp
  mymoneydbdriver.cpp
  kselectdatabasedlg.cpp
  kgeneratesqldlg.cpp
  )

ki18n_wrap_ui(sqlstorage_SOURCES kselectdatabasedlg.ui kgeneratesqldlg.ui)

add_library(sqlstorage MODULE ${sqlstorage_SOURCES} )
kde_target_enable_exceptions(sqlstorage PUBLIC)

target_link_libraries(sqlstorage
  PUBLIC
    kmm_plugin
    kmm_widgets
    kmm_utils_platformtools
)

install(FILES sqlstorage.rc
        DESTINATION "${KXMLGUI_INSTALL_DIR}/sqlstorage")

install(TARGETS sqlstorage
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney")

install(FILES kmymoney-sqlstorageplugin.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR}
)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
