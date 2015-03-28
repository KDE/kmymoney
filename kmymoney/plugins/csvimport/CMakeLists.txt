# patch the version with the version defined in the build system
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/kmm_csvimport.desktop
  ${CMAKE_CURRENT_BINARY_DIR}/kmm_csvimport.desktop
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

########### next target ###############

SET(kmm_csvimport_PART_SRCS
  csvdialog.cpp
  csvimporterplugin.cpp
  csvwizard.cpp
  investmentdlg.cpp
  investprocessing.cpp
  redefinedlg.cpp
  convdate.cpp
  csvutil.cpp
  symboltabledlg.cpp
)

KDE4_ADD_UI_FILES(kmm_csvimport_PART_SRCS
  csvdialog.ui
  csvwizard.ui
  redefinedlgdecl.ui
  introwizardpage.ui
  separatorwizardpage.ui
  bankingwizardpage.ui
  investmentwizardpage.ui
  lines-datewizardpage.ui
  completionwizardpage.ui
  symboltabledlg.ui
)

KDE4_ADD_PLUGIN(kmm_csvimport
  ${kmm_csvimport_PART_SRCS}
)

TARGET_LINK_LIBRARIES(kmm_csvimport
  kmm_mymoney kmm_plugin
  ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS}
  ${GMP_LIBRARIES}
  ${LIBALKIMIA_LIBRARY}
)

IF(KDE4_BUILD_TESTS)
  SET(csvdatetest_SRCS csvdatetest.cpp convdate.cpp)
  KDE4_ADD_UNIT_TEST(csvdatetest TESTNAME kmymoney-plugin-csvdatetest
    ${csvdatetest_SRCS})
  TARGET_LINK_LIBRARIES(csvdatetest ${QT_AND_KDECORE_LIBS}
    ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} kmm_mymoney)

  SET(parsedatatest_SRCS parsedatatest.cpp csvutil.cpp)
  KDE4_ADD_UNIT_TEST(parsedatatest TESTNAME kmymoney-plugin-parsedatatest
    ${parsedatatest_SRCS})
  TARGET_LINK_LIBRARIES(parsedatatest ${QT_AND_KDECORE_LIBS} ${KDE4_KDEUI_LIBS}
    ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${GMP_LIBRARIES}
    ${LIBALKIMIA_LIBRARY} kmm_mymoney)

  SET(symboltest_SRCS symboltest.cpp csvutil.cpp)
  KDE4_ADD_UNIT_TEST(symboltest TESTNAME kmymoney-plugin-symboltest
    ${symboltest_SRCS})
  TARGET_LINK_LIBRARIES(symboltest ${QT_AND_KDECORE_LIBS} ${KDE4_KDEUI_LIBS}
    ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${GMP_LIBRARIES}
    ${LIBALKIMIA_LIBRARY} kmm_mymoney)
ENDIF(KDE4_BUILD_TESTS)

########### install files ###############

INSTALL(FILES kmm_csvimport.rc
        DESTINATION  ${DATA_INSTALL_DIR}/kmm_csvimport)

INSTALL(TARGETS kmm_csvimport
        DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_csvimport.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

INSTALL(FILES csvimporterrc
  DESTINATION ${CONFIG_INSTALL_DIR})
