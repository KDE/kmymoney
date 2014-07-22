# patch the version with the version defined in the build system
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/kmm_icalendarexport.desktop ${CMAKE_CURRENT_BINARY_DIR}/kmm_icalendarexport.desktop)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${LIBICAL_INCLUDE_DIRS}
                    )

SET(kmm_icalendarexport_PART_SRCS
    schedulestoicalendar.cpp
    icalendarexport.cpp
    )

kconfig_add_kcfg_files(kmm_icalendarexport_PART_SRCS pluginsettings.kcfgc)

add_library(kmm_icalendarexport MODULE ${kmm_icalendarexport_PART_SRCS})

TARGET_LINK_LIBRARIES(kmm_icalendarexport
                      ${KDE4_KDEUI_LIBS}
                      KF5::KIOWidgets
                      Alkimia::alkimia
                      kmm_mymoney kmm_plugin
                      ${LIBICAL_LIBRARIES}
                      )

INSTALL(FILES kmm_icalendarexport.rc
        DESTINATION  ${DATA_INSTALL_DIR}/kmm_icalendarexport)

INSTALL(TARGETS kmm_icalendarexport
        DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_icalendarexport.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

# the KCM module

SET(kcm_kmm_icalendarexport_PART_SRCS
    kcm_icalendarexport.cpp
    )

kconfig_add_kcfg_files(kcm_kmm_icalendarexport_PART_SRCS pluginsettings.kcfgc)

qt5_wrap_ui(kcm_kmm_icalendarexport_PART_SRCS pluginsettingsdecl.ui)

add_library(kcm_kmm_icalendarexport MODULE ${kcm_kmm_icalendarexport_PART_SRCS})

TARGET_LINK_LIBRARIES(kcm_kmm_icalendarexport
                      ${KDE4_KDEUI_LIBS}
                      KF5::KIOWidgets
                      )

INSTALL(TARGETS kcm_kmm_icalendarexport
        DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES kcm_kmm_icalendarexport.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
