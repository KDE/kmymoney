configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xmlstorage.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/xmlstorage.json @ONLY)

set(xmlstorage_SOURCES
  mymoneyxmlreader.cpp
  mymoneyxmlwriter.cpp
  mymoneyanonwriter.cpp
  mymoneystoragenames.cpp
  kgpgkeyselectiondlg.cpp
  xmlstorage.cpp
  )

ki18n_wrap_ui(xmlstorage_SOURCES kgpgkeyselectiondlg.ui)

kmymoney_add_plugin(xmlstorage SOURCES ${xmlstorage_SOURCES})

target_link_libraries(xmlstorage
  PUBLIC
    kmm_plugin
  PRIVATE
    Qt::Xml
    KF${QT_MAJOR_VERSION}::Archive
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::CoreAddons
    $<$<BOOL:${ENABLE_GPG}>:kmm_gpgfile>
    kmymoney_common
    kmm_mymoney
    xmlstoragehelper
    onlinetask_interfaces
)

add_library(xmlstorage_static STATIC ${xmlstorage_SOURCES})

target_link_libraries(xmlstorage_static
  PUBLIC
    kmm_plugin
    Qt::Xml
    KF${QT_MAJOR_VERSION}::Archive
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::CoreAddons
    $<$<BOOL:${ENABLE_GPG}>:kmm_gpgfile>
    kmymoney_common
    kmm_mymoney
    xmlstoragehelper
    onlinetask_interfaces
)

# install(FILES kmymoney-xmlstorageplugin.desktop
#   DESTINATION ${SERVICETYPES_INSTALL_DIR}
# )

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()


# the KCM module
set(kcm_xmlstorage_PART_SRCS
    kcm_xmlstorage.cpp
    )

kconfig_add_kcfg_files(kcm_xmlstorage_PART_SRCS xmlstoragesettings.kcfgc)

ki18n_wrap_ui(kcm_xmlstorage_PART_SRCS xmlstoragesettings.ui)

kmymoney_add_plugin_kcm(kcm_xmlstorage SOURCES ${kcm_xmlstorage_PART_SRCS})

target_link_libraries(kcm_xmlstorage
  PRIVATE
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::CoreAddons
    KF${QT_MAJOR_VERSION}::ConfigWidgets
    KF${QT_MAJOR_VERSION}::Completion
    $<$<BOOL:${ENABLE_GPG}>:kmm_gpgfile>
    kmm_settings
)
