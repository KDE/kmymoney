include(ECMAddAppIcon)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/settings/
                    ${CMAKE_CURRENT_BINARY_DIR}/settings/
                    ${CMAKE_CURRENT_BINARY_DIR}/dialogs/
                    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/
                    ${CMAKE_CURRENT_BINARY_DIR}/widgets/
                    ${CMAKE_CURRENT_SOURCE_DIR}/mymoney/
                    ${CMAKE_CURRENT_SOURCE_DIR}/mymoney/storage/
                    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/
                    ${CMAKE_CURRENT_BINARY_DIR}/interfaces/
                    ${CMAKE_CURRENT_SOURCE_DIR}/plugins/
                    ${CMAKE_CURRENT_BINARY_DIR}/plugins/
                    ${CMAKE_CURRENT_SOURCE_DIR}/views/
                    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/
                    ${CMAKE_CURRENT_SOURCE_DIR}/converter/
                    ${CMAKE_CURRENT_BINARY_DIR}/dialogs/settings/
                    ${CMAKE_CURRENT_BINARY_DIR}/mymoney/storage/
                    ${CMAKE_CURRENT_BINARY_DIR}/mymoney/
                    ${CMAKE_CURRENT_SOURCE_DIR}/wizards/endingbalancedlg/
                    ${CMAKE_CURRENT_BINARY_DIR}/wizards/endingbalancedlg/
                    ${CMAKE_CURRENT_SOURCE_DIR}/wizards/newinvestmentwizard/
                    ${CMAKE_CURRENT_BINARY_DIR}/wizards/newinvestmentwizard/
                    ${CMAKE_CURRENT_SOURCE_DIR}/wizards/newloanwizard/
                    ${CMAKE_CURRENT_BINARY_DIR}/wizards/newloanwizard/
                    ${CMAKE_CURRENT_SOURCE_DIR}/wizards/wizardpages/
                    ${CMAKE_CURRENT_SOURCE_DIR}/models/
                    ${CMAKE_CURRENT_BINARY_DIR}/models/
                    ${CMAKE_CURRENT_SOURCE_DIR}/icons/
                    ${CMAKE_CURRENT_BINARY_DIR}/icons/
                    ${CMAKE_CURRENT_SOURCE_DIR}/menus/
                    ${CMAKE_CURRENT_BINARY_DIR}/menus/
                    ${CMAKE_CURRENT_SOURCE_DIR}/misc/
                    ${CMAKE_CURRENT_BINARY_DIR}/misc/
                    )

add_subdirectory( mymoney )
add_subdirectory( settings )
add_subdirectory( models  )
add_subdirectory( interfaces )
add_subdirectory( plugins )
add_subdirectory( widgets )
add_subdirectory( dialogs )
add_subdirectory( views )
add_subdirectory( converter )
add_subdirectory( wizards )
add_subdirectory( templates )
add_subdirectory( misc )
add_subdirectory( icons )
add_subdirectory( menus )
if(BUILD_TESTING)
  add_subdirectory( tests )
endif()

set( _HEADERS kmymoneyutils.h )

########### common code (kmymoney_common) STATIC ###############
# will be linked into kmymoney, kmymoneytest, and libkmymoney.so

set( kmymoney_common_SRCS kmymoneyutils.cpp kcreditswindow.cpp )
add_library(kmymoney_common STATIC ${kmymoney_common_SRCS})

target_link_libraries(kmymoney_common
  PUBLIC
    Qt::Core
    KF${QT_MAJOR_VERSION}::ConfigGui
    KF${QT_MAJOR_VERSION}::WidgetsAddons
    KF${QT_MAJOR_VERSION}::KIOCore
    KF${QT_MAJOR_VERSION}::KIOFileWidgets
    KF${QT_MAJOR_VERSION}::KIOWidgets
    Alkimia::alkimia
    kmm_mymoney
    kmm_webconnect
    kmm_utils_platformtools
    kmm_url
    kmm_yesno
    kmm_menuactionexchanger
    $<$<BOOL:${ENABLE_ADDRESSBOOK}>:${PIMPREFIX}::AkonadiCore>
    PRIVATE
    KF${QT_MAJOR_VERSION}::I18n
    kmm_settings
)

# must build kmymoney/transactionsortoption.h
# from transactionsortoption.ui first
add_dependencies(kmymoney_common kmm_settings)
add_dependencies(wizardpages kmm_widgets)
# add_dependencies(kmm_dialogs kmm_widgets)

if(USE_MODELTEST)
  set( kmymoney_common_LIBS ${kmymoney_common_LIBS} ${QT_QTTEST_LIBRARY})
endif(USE_MODELTEST)

# remove these generated files, they are in the way and leftovers from 5.0
if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/kmymoneysettings.h)
  file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/kmymoneysettings.h)
endif()
if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/kmymoneysettings.cpp)
  file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/kmymoneysettings.cpp)
endif()

########### kmymoney executable ###############

set( kmymoney_SRCS
  main.cpp
  kmymoney.cpp
  pluginloader.cpp
)

if(KMM_DBUS)
    qt_add_dbus_adaptor(kmymoney_SRCS org.kde.kmymoney.xml kmymoney.h KMyMoneyApp)
endif()
qt_add_resources(kmymoney_SRCS kmymoney.qrc)

# collect application icons
file(GLOB_RECURSE KMYMONEY_APP_ICONS "${CMAKE_CURRENT_SOURCE_DIR}/icons/kmymoney/apps/*.png")
# add icons to application sources, to have them bundled
ecm_add_app_icon(kmymoney_SRCS ICONS ${KMYMONEY_APP_ICONS})

add_executable( kmymoney ${kmymoney_SRCS} )
if (BUILD_STATIC_PLUGINS)
    kcoreaddons_target_static_plugins(kmymoney "kmymoney_plugins" LINK_OPTION PRIVATE)
    kcoreaddons_target_static_plugins(kmymoney "kmymoney_plugins/kcms" LINK_OPTION PRIVATE)
    if (ENABLE_SQLCIPHER)
        target_link_libraries(kmymoney PRIVATE qsqlcipher)
        target_compile_definitions(kmymoney PRIVATE -DIMPORT_SQLCIPHER_PLUGIN)
    endif()
endif()

if (ENABLE_SQLCIPHER)
  message( STATUS "     SQLCIPHER INCLUDE   :  " ${SQLCIPHER_INCLUDE_DIRS}) #just for testing it on MS Windows
  message( STATUS "     SQLCIPHER LIBRARIES :  " ${SQLCIPHER_LIBRARY_DIRS}) #just for testing it on MS Windows
  link_directories(${SQLCIPHER_LIBRARY_DIRS})

  target_compile_definitions(kmymoney
    PRIVATE
      SQLITE_HAS_CODEC
      SQLITE_TEMP_STORE=2)

  #otherwise "fatal error: 'sqlite3.h' file not found"
  if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_include_directories(kmymoney
      PRIVATE
        ${SQLCIPHER_INCLUDE_DIRS})
  endif()
endif()

target_link_libraries(kmymoney
  PUBLIC
  views
  kmymoney_common
  newuserwizard
  newaccountwizard
  newinvestmentwizard
  newloanwizard
  endingbalancedlg
  wizardpages
  kmm_extended_dialogs
  kmm_base_dialogs
  settings
  kmm_models
  kmm_settings
  kmm_menus
  kmm_widgets
  kmm_mymoney
  interfaces
  kmm_plugin
  Qt::Core
  KF${QT_MAJOR_VERSION}::ConfigGui
  KF${QT_MAJOR_VERSION}::WidgetsAddons
  KF${QT_MAJOR_VERSION}::CoreAddons
  KF${QT_MAJOR_VERSION}::Crash
  kmm_templates
  $<$<BOOL:${USE_MODELTEST}>:Qt::Test>
  $<$<BOOL:${ENABLE_HOLIDAYS}>:KF${QT_MAJOR_VERSION}::Holidays>
  $<$<BOOL:${ENABLE_ACTIVITIES}>:KF${QT_MAJOR_VERSION}::Activities>
  PRIVATE
    kmm_selections
    $<$<BOOL:${ENABLE_SQLCIPHER}>:PkgConfig::SQLCIPHER>
)

# own plist magic for mac os
if(APPLE)
    string(TIMESTAMP CURRENT_YEAR "%Y")
    # own plist template
    set_target_properties(kmymoney PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/MacOSXBundleInfo.plist.in)

    # the MacOSX bundle display name property (CFBundleDisplayName) is not currently supported by cmake,
    # so has to be set for all targets in this cmake file
    set(MACOSX_BUNDLE_DISPLAY_NAME KMyMoney)
    set_target_properties(kmymoney PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.KMyMoney")
    set_target_properties(kmymoney PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KMyMoney")
    set_target_properties(kmymoney PROPERTIES MACOSX_BUNDLE_DISPLAY_NAME "KMyMoney")
    set_target_properties(kmymoney PROPERTIES MACOSX_BUNDLE_INFO_STRING "KMyMoney - Personal Finances Manager")
    set_target_properties(kmymoney PROPERTIES MACOSX_BUNDLE_SHORT_VERSION_STRING "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
    set_target_properties(kmymoney PROPERTIES MACOSX_BUNDLE_BUNDLE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
    set_target_properties(kmymoney PROPERTIES MACOSX_BUNDLE_COPYRIGHT "© 2000-${CURRENT_YEAR} The KMyMoney Authors")
endif()

########### install files ###############

install(TARGETS kmymoney ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES ${_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kmymoney
  )
install(FILES org.kde.kmymoney.desktop
  DESTINATION ${KDE_INSTALL_APPDIR}
  )
install(FILES org.kde.kmymoney.appdata.xml
  DESTINATION ${KDE_INSTALL_METAINFODIR}
  )

install(FILES x-kmymoney.xml
  DESTINATION ${KDE_INSTALL_MIMEDIR})

#UPDATE_XDG_MIMETYPES(${XDG_MIME_INSTALL_DIR})


