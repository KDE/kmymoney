# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmm_kbanking.desktop ${CMAKE_CURRENT_BINARY_DIR}/kmm_kbanking.desktop)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
                    ${CMAKE_CURRENT_BINARY_DIR}/dialogs
                    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
                    ${CMAKE_CURRENT_BINARY_DIR}/widgets
                    ${GWENHYWFAR_INCLUDE_DIRS}
                    ${AQBANKING_INCLUDE_DIRS})

add_subdirectory( dialogs )
add_subdirectory( widgets )
add_subdirectory( qml )
add_subdirectory( tasksettings )

########### next target ###############

set(kmm_kbanking_PART_SRCS
    mymoneybanking.cpp
    banking.cpp
    aqbankingkmmoperators.cpp
    gwenhywfarqtoperators.cpp
    gwenkdegui.cpp
)

kconfig_add_kcfg_files(kmm_kbanking_PART_SRCS kbankingsettings.kcfgc)

add_library(kmm_kbanking ${kmm_kbanking_PART_SRCS})

kcoreaddons_desktop_to_json(kmm_kbanking kmm_kbanking.desktop)

target_link_libraries(kmm_kbanking
                      kmm_kbanking_dialogs
                      kmm_kbanking_widgets
                      kmm_kbanking_tasksettings
                      kmm_mymoney
                      kmm_widgets
                      kmm_plugin
                      KF5::ConfigCore
                      KF5::KDELibs4Support
                      payeeidentifier_nationalAccount
                      ${GWENHYWFAR_LIBRARIES}
                      ${AQBANKING_LIBRARIES}
                      Alkimia::alkimia
)


########### install files ###############

install(FILES kmm_kbanking.rc
        DESTINATION  ${KXMLGUI_INSTALL_DIR}/kmm_kbanking)

install(TARGETS kmm_kbanking
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_kbanking.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES kbanking.kcfg
        DESTINATION ${KCFG_INSTALL_DIR})
