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
KDE4_ADD_KCFG_FILES( kmm_config_SRCS kmymoneysettings.kcfgc )
KDE4_ADD_LIBRARY(kmm_config STATIC ${kmm_config_SRCS})

########### common code (kmymoney_common) STATIC ###############
# will be linked into kmymoney, kmymoneytest, and libkmymoney.so

SET( kmymoney_common_SRCS kmymoneyutils.cpp kstartuplogo.cpp )
KDE4_ADD_LIBRARY(kmymoney_common STATIC ${kmymoney_common_SRCS})
TARGET_LINK_LIBRARIES(kmymoney_common
  ${QT_AND_KDECORE_LIBS} ${KDEPIMLIBS_KHOLIDAYS_LIBS} ${GMP_LIBRARIES} ${LIBALKIMIA_LIBRARY} kmm_widgets kmm_mymoney)

# must build kmymoney/transactionsortoption.h
# from transactionsortoption.ui first
ADD_DEPENDENCIES(kmymoney_common generate_base_ui_srcs kmm_config)

SET( kmymoney_common_LIBS
  views reports kmymoney_base kmymoney_common
  newuserwizard newaccountwizard newinvestmentwizard newloanwizard
  endingbalancedlg wizardpages
  dialogs widgets settings converter models kmm_config
  kmm_storage kmm_mymoney kgpgfile interfaces kmm_plugin dialogs kmm_kdchart
  ${QT_AND_KDECORE_LIBS} ${QT_QTSQL_LIBRARY}
  ${KDE4_KHTML_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KABC_LIBS} ${KDE4_KFILE_LIBS} ${KDE4_KUTILS_LIBRARY})

########### kmymoney executable ###############

SET( kmymoney_SRCS main.cpp kmymoney.cpp )

QT4_ADD_DBUS_ADAPTOR(kmymoney_SRCS org.kde.kmymoney.xml kmymoney.h KMyMoneyApp)
KDE4_ADD_APP_ICON( kmymoney_SRCS hi*-app-kmymoney.png )

KDE4_ADD_EXECUTABLE( kmymoney ${kmymoney_SRCS} )
TARGET_LINK_LIBRARIES( kmymoney ${kmymoney_common_LIBS} )

########### install files ###############

INSTALL(TARGETS kmymoney ${INSTALL_TARGETS_DEFAULT_ARGS})

INSTALL(FILES kmymoney.kcfg
  DESTINATION ${KCFG_INSTALL_DIR}
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

INSTALL(FILES x-kmymoney.xml
  DESTINATION ${XDG_MIME_INSTALL_DIR})

UPDATE_XDG_MIMETYPES(${XDG_MIME_INSTALL_DIR})

########## Application Icons #############
INSTALL(DIRECTORY icons/oxygen
  DESTINATION ${DATA_INSTALL_DIR}/kmymoney/icons
  PATTERN "*Make*" EXCLUDE
  PATTERN ".svn" EXCLUDE
  )
INSTALL(FILES hi16-app-kmymoney.png
  DESTINATION ${ICON_INSTALL_DIR}/hicolor/16x16/apps
  RENAME kmymoney.png
  )
INSTALL(FILES mimetype_kmy_16x16.png
  DESTINATION ${ICON_INSTALL_DIR}/hicolor/16x16/mimetypes
  RENAME kmy.png
  )
INSTALL(FILES hi32-app-kmymoney.png
  DESTINATION ${ICON_INSTALL_DIR}/hicolor/32x32/apps
  RENAME kmymoney.png
  )
INSTALL(FILES mimetype_kmy_22x22.png
  DESTINATION ${ICON_INSTALL_DIR}/hicolor/22x22/mimetypes
  RENAME kmy.png
  )
INSTALL(FILES mimetype_kmy_32x32.png
  DESTINATION ${ICON_INSTALL_DIR}/hicolor/32x32/mimetypes
  RENAME kmy.png
  )
INSTALL(FILES hi48-app-kmymoney.png
  DESTINATION ${ICON_INSTALL_DIR}/hicolor/48x48/apps
  RENAME kmymoney.png
  )
INSTALL(FILES mimetype_kmy_48x48.png
  DESTINATION ${ICON_INSTALL_DIR}/hicolor/48x48/mimetypes
  RENAME kmy.png
  )
INSTALL(FILES hi64-app-kmymoney.png
  DESTINATION ${ICON_INSTALL_DIR}/hicolor/64x64/apps
  RENAME kmymoney.png
  )
INSTALL(FILES mimetype_kmy_64x64.png
  DESTINATION ${ICON_INSTALL_DIR}/hicolor/64x64/mimetypes
  RENAME kmy.png
  )
INSTALL(FILES hi128-app-kmymoney.png
  DESTINATION ${ICON_INSTALL_DIR}/hicolor/128x128/apps
  RENAME kmymoney.png
  )
INSTALL(FILES mimetype_kmy_128x128.png
  DESTINATION ${ICON_INSTALL_DIR}/hicolor/128x128/mimetypes
  RENAME kmy.png
  )
INSTALL(FILES lo16-app-kmymoney.png
  DESTINATION ${ICON_INSTALL_DIR}/locolor/16x16/apps
  RENAME kmymoney.png
  )
INSTALL(FILES lo32-app-kmymoney.png
  DESTINATION ${ICON_INSTALL_DIR}/locolor/32x32/apps
  RENAME kmymoney.png
  )

############## tests ####################
IF( KDE4_BUILD_TESTS )

  SET(target_link_libraries_test
	widgets
	kmm_mymoney
	kmymoney_common
	dialogs
	widgets
	kmm_config
	${KDE4_KHTML_LIBS}
	${KDE4_KDEUI_LIBS}
	${KDE4_KABC_LIBS}
	${KDE4_KFILE_LIBS}
	${KDE4_KUTILS_LIBRARY}
        ${QT_QTGUI_LIBRARY}
        ${QT_QTXML_LIBRARY}
        ${QT_QTCORE_LIBRARY}
	${QT_QTSQL_LIBRARY}
        ${QT_QTTEST_LIBRARY}
        ${GMP_LIBRARIES}
        ${LIBALKIMIA_LIBRARY})

  # KMyMoneyUtils
  SET( kmymoneyutilstest_SRCS kmymoneyutilstest.cpp )

  KDE4_ADD_UNIT_TEST( kmymoneyutilstest TESTNAME kmymoney-utilstest ${kmymoneyutilstest_SRCS} )
  ADD_DEPENDENCIES( kmymoneyutilstest kmymoney_common kmm_mymoney )
  TARGET_LINK_LIBRARIES( kmymoneyutilstest ${target_link_libraries_test})

ENDIF( KDE4_BUILD_TESTS )

