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
  investmentdlg.cpp
  investprocessing.cpp
  redefinedlg.cpp
  convdate.cpp
  csvutil.cpp
  symboltabledlg.cpp
)

qt5_wrap_ui(kmm_csvimport_PART_SRCS
  csvdialog.ui
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
  ${KDE4_KDEUI_LIBS} KF5::KIOWidgets
  ${GMP_LIBRARIES}
  Alkimia::alkimia
)

IF(KDE4_BUILD_TESTS)
  SET(csvdatetest_SRCS csvdatetest.cpp convdate.cpp)
  KDE4_ADD_UNIT_TEST(csvdatetest TESTNAME kmymoney-plugin-csvdatetest
    ${csvdatetest_SRCS})
  TARGET_LINK_LIBRARIES(csvdatetest ${QT_AND_KDECORE_LIBS}
    Qt5::Test  kmm_mymoney)

  SET(parsedatatest_SRCS parsedatatest.cpp csvutil.cpp)
  KDE4_ADD_UNIT_TEST(parsedatatest TESTNAME kmymoney-plugin-parsedatatest
    ${parsedatatest_SRCS})
  TARGET_LINK_LIBRARIES(parsedatatest ${QT_AND_KDECORE_LIBS} ${KDE4_KDEUI_LIBS}
    Qt5::Test  ${GMP_LIBRARIES}
    Alkimia::alkimia kmm_mymoney)

  SET(symboltest_SRCS symboltest.cpp csvutil.cpp)
  KDE4_ADD_UNIT_TEST(symboltest TESTNAME kmymoney-plugin-symboltest
    ${symboltest_SRCS})
  TARGET_LINK_LIBRARIES(symboltest ${QT_AND_KDECORE_LIBS} ${KDE4_KDEUI_LIBS}
    Qt5::Test  ${GMP_LIBRARIES}
    Alkimia::alkimia kmm_mymoney)
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
