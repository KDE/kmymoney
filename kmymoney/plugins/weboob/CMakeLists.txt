# patch the version with the version defined in the build system
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/kmm_weboob.desktop ${CMAKE_CURRENT_BINARY_DIR}/kmm_weboob.desktop)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
                    ${CMAKE_CURRENT_BINARY_DIR}/dialogs
                    ${KROSS_INCLUDES})

ADD_SUBDIRECTORY( dialogs )

########### next target ###############

SET(kmm_weboob_PART_SRCS
    weboob.cpp
    plugin.cpp
    )

KDE4_ADD_PLUGIN(kmm_weboob ${kmm_weboob_PART_SRCS})

TARGET_LINK_LIBRARIES(kmm_weboob
                      kmm_weboob_dialogs
                      kmm_mymoney
                      kmm_widgets
                      kmm_plugin
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KROSSCORE_LIBS}
                      ${LIBALKIMIA_LIBRARY}
                      )

########### install files ###############

INSTALL(FILES weboob.py kmm_weboob.rc
        DESTINATION  ${DATA_INSTALL_DIR}/kmm_weboob)

INSTALL(TARGETS kmm_weboob
        DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_weboob.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
