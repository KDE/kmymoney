INCLUDE_DIRECTORIES( ${LIBALKIMIA_INCLUDE_DIR} )

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${KDEPIMLIBS_INCLUDE_DIRS}
                    ${CMAKE_CURRENT_BINARY_DIR}/dialogs/
                    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/
                    ${CMAKE_CURRENT_BINARY_DIR}/widgets/
                    ${CMAKE_CURRENT_SOURCE_DIR}/mymoney/
                    ${CMAKE_CURRENT_SOURCE_DIR}/mymoney/storage/
                    ${CMAKE_CURRENT_SOURCE_DIR}/plugins/
                    ${CMAKE_CURRENT_SOURCE_DIR}/views/
                    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/
                    ${CMAKE_CURRENT_SOURCE_DIR}/converter/
                    ${CMAKE_CURRENT_BINARY_DIR}/dialogs/settings/
                    ${CMAKE_CURRENT_BINARY_DIR}/mymoney/storage/
                    ${CMAKE_CURRENT_SOURCE_DIR}/reports/
                    ${CMAKE_CURRENT_SOURCE_DIR}/wizards/endingbalancedlg/
                    ${CMAKE_CURRENT_BINARY_DIR}/wizards/endingbalancedlg/
                    ${CMAKE_CURRENT_SOURCE_DIR}/wizards/newinvestmentwizard/
                    ${CMAKE_CURRENT_BINARY_DIR}/wizards/newinvestmentwizard/
                    ${CMAKE_CURRENT_SOURCE_DIR}/wizards/newloanwizard/
                    ${CMAKE_CURRENT_BINARY_DIR}/wizards/newloanwizard/
                    ${CMAKE_CURRENT_SOURCE_DIR}/wizards/wizardpages/
                    ${CMAKE_CURRENT_SOURCE_DIR}/models/
                    ${CMAKE_CURRENT_BINARY_DIR}/models/
                    ${KMyMoney_SOURCE_DIR}/libkgpgfile/
                    ${KMyMoney_SOURCE_DIR}/libkdchart/include )

ADD_SUBDIRECTORY( mymoney )
ADD_SUBDIRECTORY( models  )
ADD_SUBDIRECTORY( plugins )
ADD_SUBDIRECTORY( reports )
ADD_SUBDIRECTORY( widgets )
ADD_SUBDIRECTORY( dialogs )
ADD_SUBDIRECTORY( views )
ADD_SUBDIRECTORY( converter )
ADD_SUBDIRECTORY( wizards )
ADD_SUBDIRECTORY( pics )
ADD_SUBDIRECTORY( html )
ADD_SUBDIRECTORY( templates )
ADD_SUBDIRECTORY( misc )

SET( _HEADERS kmymoneyutils.h kmymoneyglobalsettings.h )

########### settings code (kmm_config) STATIC ###############
SET( kmm_config_SRCS kmymoneyglobalsettings.cpp )
kconfig_add_kcfg_files( kmm_config_SRCS kmymoneysettings.kcfgc )
add_library(kmm_config STATIC ${kmm_config_SRCS})
target_link_libraries(kmm_config Alkimia::alkimia)

########### common code (kmymoney_common) STATIC ###############
# will be linked into kmymoney, kmymoneytest, and libkmymoney.so

SET( kmymoney_common_SRCS kmymoneyutils.cpp kstartuplogo.cpp )
add_library(kmymoney_common STATIC ${kmymoney_common_SRCS})
TARGET_LINK_LIBRARIES(kmymoney_common
  Qt5::Core KF5::ConfigGui KF5::WidgetsAddons ${GMP_LIBRARIES} Alkimia::alkimia kmm_widgets kmm_mymoney)

# must build kmymoney/transactionsortoption.h
# from transactionsortoption.ui first
ADD_DEPENDENCIES(kmymoney_common generate_base_ui_srcs kmm_config)
ADD_DEPENDENCIES(wizardpages widgets)

SET( kmymoney_common_LIBS
  views reports kmymoney_base kmymoney_common
  newuserwizard newaccountwizard newinvestmentwizard newloanwizard
  endingbalancedlg wizardpages
  dialogs widgets settings converter models kmm_config
  kmm_storage kmm_mymoney kgpgfile interfaces kmm_plugin dialogs kmm_kdchart
  Qt5::Core Qt5::Sql
  KF5::KHtml KF5::Archive KF5::Holidays KF5::ConfigGui KF5::WidgetsAddons)

########### kmymoney executable ###############

SET( kmymoney_SRCS main.cpp kmymoney.cpp )

qt5_add_dbus_adaptor(kmymoney_SRCS org.kde.kmymoney.xml kmymoney.h KMyMoneyApp)
#KDE4_ADD_APP_ICON( kmymoney_SRCS hi*-app-kmymoney.png )

add_executable( kmymoney ${kmymoney_SRCS} )
TARGET_LINK_LIBRARIES( kmymoney ${kmymoney_common_LIBS} )

########### install files ###############

INSTALL(TARGETS kmymoney ${INSTALL_TARGETS_DEFAULT_ARGS})

INSTALL(FILES kmymoney.kcfg
  DESTINATION ${KCFG_INSTALL_DIR}
  )
INSTALL(FILES kmymoney.upd
  DESTINATION ${KCONF_UPDATE_INSTALL_DIR}
  )
INSTALL(FILES kmymoneyui.rc tips
  DESTINATION ${DATA_INSTALL_DIR}/kmymoney
  )
INSTALL(FILES ${_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney
  )
INSTALL(PROGRAMS kmymoney.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR}
  )
INSTALL(PROGRAMS kmymoney.appdata.xml
  DESTINATION share/appdata/
  )

INSTALL(FILES x-kmymoney.xml
  DESTINATION ${XDG_MIME_INSTALL_DIR})

#UPDATE_XDG_MIMETYPES(${XDG_MIME_INSTALL_DIR})

include(ECMInstallIcons)

########## Application Icons #############
INSTALL(DIRECTORY icons/hicolor
  DESTINATION ${DATA_INSTALL_DIR}/kmymoney/icons
  PATTERN "*Make*" EXCLUDE
  PATTERN ".svn" EXCLUDE
  )
INSTALL(DIRECTORY icons/oxygen
  DESTINATION ${DATA_INSTALL_DIR}/kmymoney/icons
  PATTERN "*Make*" EXCLUDE
  PATTERN ".svn" EXCLUDE
  )
ecm_install_icons(${ICON_INSTALL_DIR})

############## tests ####################
include(ECMMarkAsTest)

IF( KDE4_BUILD_TESTS )

  SET(target_link_libraries_test
        kmm_mymoney
        kmymoney_common
        kmm_config
        widgets
        KF5::KHtml KF5::Archive KF5::Holidays KF5::ConfigGui KF5::WidgetsAddons
        Qt5::Core Qt5::Sql Qt5::Test
        Alkimia::alkimia)

  # KMyMoneyUtils
  SET( kmymoneyutilstest_SRCS kmymoneyutilstest.cpp )

  add_executable( kmymoneyutilstest ${kmymoneyutilstest_SRCS} )
  ecm_mark_as_test(kmymoneyutilstest)
  add_test(kmymoney-utilstest kmymoneyutilstest)

  ADD_DEPENDENCIES( kmymoneyutilstest kmymoney_common kmm_mymoney )
  TARGET_LINK_LIBRARIES( kmymoneyutilstest ${target_link_libraries_test})

ENDIF( KDE4_BUILD_TESTS )

