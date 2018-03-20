include_directories( ${LIBALKIMIA_INCLUDE_DIR} )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${KDEPIMLIBS_INCLUDE_DIRS}
                    ${CMAKE_CURRENT_BINARY_DIR}/dialogs/
                    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/
                    ${CMAKE_CURRENT_BINARY_DIR}/widgets/
                    ${CMAKE_CURRENT_BINARY_DIR}/settings/
                    ${CMAKE_CURRENT_SOURCE_DIR}/mymoney/
                    ${CMAKE_CURRENT_SOURCE_DIR}/mymoney/storage/
                    ${CMAKE_CURRENT_SOURCE_DIR}/plugins/
                    ${CMAKE_CURRENT_BINARY_DIR}/plugins/
                    ${CMAKE_CURRENT_SOURCE_DIR}/views/
                    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/
                    ${CMAKE_CURRENT_SOURCE_DIR}/converter/
                    ${CMAKE_CURRENT_BINARY_DIR}/dialogs/settings/
                    ${CMAKE_CURRENT_BINARY_DIR}/mymoney/storage/
                    ${CMAKE_CURRENT_BINARY_DIR}/mymoney/
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
                    ${CMAKE_CURRENT_BINARY_DIR}/payeeidentifier/ibanandbic/widgets/ # TODO: this line should be moved to the target it belongs
                    ${CMAKE_CURRENT_BINARY_DIR}/payeeidentifier/ibanandbic/
                    ${CMAKE_CURRENT_BINARY_DIR}/payeeidentifier/nationalaccount/
                    ${KMyMoney_SOURCE_DIR}/libkgpgfile/
                    ${KMyMoney_SOURCE_DIR}/libkdchart/include/KDChart )

add_subdirectory( settings )
add_subdirectory( mymoney )
add_subdirectory( models  )
add_subdirectory( plugins )
add_subdirectory( reports )
add_subdirectory( widgets )
add_subdirectory( dialogs )
add_subdirectory( views )
add_subdirectory( converter )
add_subdirectory( wizards )
add_subdirectory( pics )
add_subdirectory( html )
add_subdirectory( templates )
add_subdirectory( misc )
add_subdirectory( payeeidentifier )

set( _HEADERS kmymoneyutils.h kmymoneyglobalsettings.h )
########### common code (kmymoney_common) STATIC ###############
# will be linked into kmymoney, kmymoneytest, and libkmymoney.so

set( kmymoney_common_SRCS
  kmymoneyutils.cpp
  kstartuplogo.cpp
  kmymoneyglobalsettings.cpp
)
kde4_add_library(kmymoney_common STATIC ${kmymoney_common_SRCS})
target_link_libraries(kmymoney_common
  ${QT_AND_KDECORE_LIBS} ${KDEPIMLIBS_KHOLIDAYS_LIBS} ${LIBALKIMIA_LIBRARIES} kmm_widgets kmm_mymoney)

# must build kmymoney/transactionsortoption.h
# from transactionsortoption.ui first
add_dependencies(kmymoney_common generate_base_ui_srcs kmm_settings)
add_dependencies(wizardpages widgets)

set( kmymoney_common_LIBS
  views reports kmymoney_base kmymoney_common
  newuserwizard newaccountwizard newinvestmentwizard newloanwizard
  endingbalancedlg wizardpages
  dialogs widgets settings converter models kmm_settings
  kmm_widgets
  kmm_storage kmm_mymoney kgpgfile interfaces kmm_plugin dialogs kmm_kdchart
  ${QT_AND_KDECORE_LIBS} ${QT_QTSQL_LIBRARY}
  ${KDE4_KHTML_LIBS} ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_KPIMIDENTITIES_LIBS} ${KDEPIMLIBS_AKONADI_CONTACT_LIBS} ${KDE4_KFILE_LIBS} ${KDE4_KUTILS_LIBRARY}
  )

########### kmymoney executable ###############

set( kmymoney_SRCS main.cpp kmymoney.cpp )

qt4_add_dbus_adaptor(kmymoney_SRCS org.kde.kmymoney.xml kmymoney.h KMyMoneyApp)
kde4_add_app_icon( kmymoney_SRCS hi*-app-kmymoney.png )

kde4_add_executable( kmymoney ${kmymoney_SRCS} )
target_link_libraries( kmymoney ${kmymoney_common_LIBS} )

########### install files ###############

install(TARGETS kmymoney ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES kmymoney.upd
  DESTINATION ${KCONF_UPDATE_INSTALL_DIR}
  )
install(FILES kmymoneyui.rc tips
  DESTINATION ${DATA_INSTALL_DIR}/kmymoney
  )
install(FILES ${_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney
  )
install(FILES org.kde.kmymoney.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR}
  )
install(FILES org.kde.kmymoney.appdata.xml
  DESTINATION ${SHARE_INSTALL_PREFIX}/appdata/
  )

install(FILES x-kmymoney.xml
  DESTINATION ${XDG_MIME_INSTALL_DIR})

update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})

########## Application Icons #############
install(DIRECTORY icons/hicolor
  DESTINATION ${DATA_INSTALL_DIR}/kmymoney/icons
  PATTERN "*Make*" EXCLUDE
  PATTERN ".svn" EXCLUDE
  )
install(DIRECTORY icons/oxygen
  DESTINATION ${DATA_INSTALL_DIR}/kmymoney/icons
  PATTERN "*Make*" EXCLUDE
  PATTERN ".svn" EXCLUDE
  )
kde4_install_icons(${ICON_INSTALL_DIR})

############## tests ####################
if( KDE4_BUILD_TESTS )

  set(target_link_libraries_test
        kmm_mymoney
        kmymoney_common
        kmm_settings
        widgets
        ${KDE4_KHTML_LIBS}
        ${KDE4_KDEUI_LIBS}
        ${KDE4_KFILE_LIBS}
        ${KDE4_KUTILS_LIBRARY}
        ${QT_QTGUI_LIBRARY}
        ${QT_QTXML_LIBRARY}
        ${QT_QTCORE_LIBRARY}
        ${QT_QTSQL_LIBRARY}
        ${QT_QTTEST_LIBRARY}
        ${LIBALKIMIA_LIBRARY}
        )

  # KMyMoneyUtils
  set( kmymoneyutilstest_SRCS kmymoneyutilstest.cpp )

  kde4_add_unit_test( kmymoneyutilstest TESTNAME kmymoney-utilstest ${kmymoneyutilstest_SRCS} )
  add_dependencies( kmymoneyutilstest kmymoney_common kmm_mymoney )
  target_link_libraries( kmymoneyutilstest ${target_link_libraries_test})

endif( KDE4_BUILD_TESTS )

