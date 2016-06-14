# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmm_kbanking.desktop ${CMAKE_CURRENT_BINARY_DIR}/kmm_kbanking.desktop)

add_subdirectory( dialogs )
add_subdirectory( widgets )
add_subdirectory( qml )
add_subdirectory( tasksettings )

set(kmm_kbanking_PART_SRCS
    mymoneybanking.cpp
    banking.cpp
    aqbankingkmmoperators.cpp
    gwenhywfarqtoperators.cpp
    gwenkdegui.cpp
)

kconfig_add_kcfg_files(kmm_kbanking_PART_SRCS kbankingsettings.kcfgc)

add_library(kmm_kbanking MODULE ${kmm_kbanking_PART_SRCS})

kcoreaddons_desktop_to_json(kmm_kbanking kmm_kbanking.desktop)

target_link_libraries(kmm_kbanking
                      kmm_kbanking_dialogs
                      kmm_kbanking_widgets
                      kmm_kbanking_tasksettings
                      kmm_mymoney
                      kmm_widgets
                      kmm_plugin
                      payeeidentifier_nationalAccount
                      KF5::ConfigCore
                      KF5::ConfigGui
                      Qt5::Core
                      Alkimia::alkimia
                      gwenhywfar::core
                      gwenhywfar::gui-cpp
                      gwenhywfar::gui-qt5
                      aqbanking::aqbanking
)
# C++11 features:
# intitializer list

# Add new menu entries if debugging is enabled
set(KMM_BANKING_DEBUG_OPTIONS)
# preprocessor definitions in case this is a debug build
string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_TOLOWER)
if("${CMAKE_BUILD_TYPE_TOLOWER}" MATCHES "^debug(full)?$")
  file(READ kmm_kbanking_debug.rc KMM_BANKING_DEBUG_OPTIONS)
endif()
configure_file(kmm_kbanking.rc "${CMAKE_CURRENT_BINARY_DIR}/kmm_kbanking.rc" @ONLY)

########### install files ###############

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_kbanking.rc
        DESTINATION ${KXMLGUI_INSTALL_DIR}/kmm_kbanking)

install(TARGETS kmm_kbanking
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_kbanking.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES kbanking.kcfg
        DESTINATION ${KCFG_INSTALL_DIR})
