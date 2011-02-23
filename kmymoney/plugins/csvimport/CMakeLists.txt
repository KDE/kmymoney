# patch the version with the version defined in the build system
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/kmm_csvimport.desktop
  ${CMAKE_CURRENT_BINARY_DIR}/kmm_csvimport.desktop
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

########### next target ###############

SET(kmm_csvimport_PART_SRCS
  csvimporterplugin.cpp
  csvimporterdlg.cpp
  csvprocessing.cpp
  investmentdlg.cpp
  investprocessing.cpp
  redefinedlg.cpp
  convdate.cpp
  csvutil.cpp
)

KDE4_ADD_UI_FILES(kmm_csvimport_PART_SRCS
  csvimporterdlgdecl.ui
  redefinedlgdecl.ui
)

KDE4_ADD_PLUGIN(kmm_csvimport
  ${kmm_csvimport_PART_SRCS}
)

TARGET_LINK_LIBRARIES(kmm_csvimport
  kmm_mymoney kmm_plugin
  ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS}
)

IF(KDE4_BUILD_TESTS)
  SET(csvdatetest_SRCS csvdatetest.cpp convdate.cpp)
  KDE4_ADD_UNIT_TEST(csvdatetest TESTNAME kmymoney-plugin-csvdatetest
    ${csvdatetest_SRCS})
  TARGET_LINK_LIBRARIES(csvdatetest ${QT_AND_KDECORE_LIBS}
    ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} )

  SET(parsedatatest_SRCS parsedatatest.cpp csvutil.cpp)
  KDE4_ADD_UNIT_TEST(parsedatatest TESTNAME kmymoney-plugin-parsedatatest
    ${parsedatatest_SRCS})
  TARGET_LINK_LIBRARIES(parsedatatest ${QT_AND_KDECORE_LIBS}
    ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} )
ENDIF(KDE4_BUILD_TESTS)

########### install files ###############

INSTALL(FILES kmm_csvimport.rc
        DESTINATION  ${DATA_INSTALL_DIR}/kmm_csvimport)

INSTALL(TARGETS kmm_csvimport
        DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_csvimport.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

