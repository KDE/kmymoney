# patch the version with the version defined in the build system
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/kmm_kbanking.desktop ${CMAKE_CURRENT_BINARY_DIR}/kmm_kbanking.desktop)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
                    ${CMAKE_CURRENT_BINARY_DIR}/dialogs
                    ${CMAKE_CURRENT_SOURCE_DIR}/views
                    ${CMAKE_CURRENT_BINARY_DIR}/views
                    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
                    ${CMAKE_CURRENT_BINARY_DIR}/widgets
                    ${GWENHYWFAR_INCLUDE_DIRS}
                    ${AQBANKING_INCLUDE_DIRS})

ADD_SUBDIRECTORY( dialogs )
ADD_SUBDIRECTORY( views )
ADD_SUBDIRECTORY( widgets )
ADD_SUBDIRECTORY( qml )
ADD_SUBDIRECTORY( tasksettings )

########### next target ###############

SET(kmm_kbanking_PART_SRCS
    mymoneybanking.cpp
    banking.cpp
    aqbankingkmmoperators.cpp
    gwenhywfarqtoperators.cpp
    gwenkdegui.cpp
    )

SET(kmm_kbanking_HEADERS
    aqbankingqtoperators.h
    gwenhywfarqtoperators.h
)

KDE4_ADD_KCFG_FILES(kmm_kbanking_settings_SRCS kbankingsettings.kcfgc)
KDE4_ADD_LIBRARY(kmm_kbanking_settings STATIC ${kmm_kbanking_settings_SRCS})

ADD_DEPENDENCIES(kmm_kbanking_widgets kmm_kbanking_settings)

KDE4_ADD_PLUGIN(kmm_kbanking ${kmm_kbanking_PART_SRCS})

TARGET_LINK_LIBRARIES(kmm_kbanking
                      kmm_kbanking_dialogs
                      kmm_kbanking_views
                      kmm_kbanking_widgets
                      kmm_kbanking_settings
                      kmm_kbanking_tasksettings
                      kmm_mymoney
                      kmm_widgets
                      kmm_plugin
                      payeeidentifier_iban_bic
                      payeeidentifier_nationalAccount
                      ${KDE4_KDEUI_LIBS}
                      ${QT_QTDECLARATIVE_LIBRARY}
                      ${GWENHYWFAR_LIBRARIES}
                      ${AQBANKING_LIBRARIES}
                      #${GMP_LIBRARIES}
                      ${LIBALKIMIA_LIBRARY}
                      )


########### install files ###############

INSTALL(FILES kmm_kbanking.rc
        DESTINATION  ${DATA_INSTALL_DIR}/kmm_kbanking)

INSTALL(TARGETS kmm_kbanking
        DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_kbanking.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

INSTALL(FILES kbanking.kcfg
        DESTINATION ${KCFG_INSTALL_DIR})
