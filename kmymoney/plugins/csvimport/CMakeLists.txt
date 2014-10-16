# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmm_csvimport.desktop
  ${CMAKE_CURRENT_BINARY_DIR}/kmm_csvimport.desktop
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

########### next target ###############

set(kmm_csvimport_PART_SRCS
  csvdialog.cpp
  csvimporterplugin.cpp
  investmentdlg.cpp
  investprocessing.cpp
  redefinedlg.cpp
  convdate.cpp
  csvutil.cpp
  symboltabledlg.cpp
)

kde4_add_ui_files(kmm_csvimport_PART_SRCS
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

kde4_add_plugin(kmm_csvimport
  ${kmm_csvimport_PART_SRCS}
)

target_link_libraries(kmm_csvimport
  kmm_mymoney kmm_plugin
  ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS}
  ${GMP_LIBRARIES}
  ${LIBALKIMIA_LIBRARY}
)

if(KDE4_BUILD_TESTS)
  set(csvdatetest_SRCS csvdatetest.cpp convdate.cpp)
  kde4_add_unit_test(csvdatetest TESTNAME kmymoney-plugin-csvdatetest
    ${csvdatetest_SRCS})
  target_link_libraries(csvdatetest ${QT_AND_KDECORE_LIBS}
    ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} kmm_mymoney)

  set(parsedatatest_SRCS parsedatatest.cpp csvutil.cpp)
  kde4_add_unit_test(parsedatatest TESTNAME kmymoney-plugin-parsedatatest
    ${parsedatatest_SRCS})
  target_link_libraries(parsedatatest ${QT_AND_KDECORE_LIBS} ${KDE4_KDEUI_LIBS}
    ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${GMP_LIBRARIES}
    ${LIBALKIMIA_LIBRARY} kmm_mymoney)

  set(symboltest_SRCS symboltest.cpp csvutil.cpp)
  kde4_add_unit_test(symboltest TESTNAME kmymoney-plugin-symboltest
    ${symboltest_SRCS})
  target_link_libraries(symboltest ${QT_AND_KDECORE_LIBS} ${KDE4_KDEUI_LIBS}
    ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${GMP_LIBRARIES}
    ${LIBALKIMIA_LIBRARY} kmm_mymoney)
endif((KDE4_BUILD_TESTS)

########### install files ###############

install(FILES kmm_csvimport.rc
        DESTINATION  ${DATA_INSTALL_DIR}/kmm_csvimport)

install(TARGETS kmm_csvimport
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_csvimport.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES csvimporterrc
  DESTINATION ${CONFIG_INSTALL_DIR})
