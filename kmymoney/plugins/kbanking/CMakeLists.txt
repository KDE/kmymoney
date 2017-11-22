# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kbanking.json.in ${CMAKE_CURRENT_BINARY_DIR}/kbanking.json @ONLY)

add_subdirectory( dialogs )
add_subdirectory( widgets )
add_subdirectory( qml )
add_subdirectory( tasksettings )

set(kmm_kbanking_PART_SRCS
    mymoneybanking.cpp
    ../../mymoney/onlinejobmessage.cpp
    banking.cpp
    aqbankingkmmoperators.cpp
    gwenhywfarqtoperators.cpp
    gwenkdegui.cpp
)

kconfig_add_kcfg_files(kmm_kbanking_PART_SRCS kbankingsettings.kcfgc)

add_library(kmm_kbanking MODULE ${kmm_kbanking_PART_SRCS})

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

# List might not be complete
target_compile_features(kmm_kbanking PRIVATE cxx_generalized_initializers cxx_nullptr)

# Add new menu entries if debugging is enabled
set(KMM_BANKING_DEBUG_OPTIONS)
# preprocessor definitions in case this is a debug build
string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_TOLOWER)
if("${CMAKE_BUILD_TYPE_TOLOWER}" MATCHES "debug")
  file(READ kmm_kbanking_debug.rc KMM_BANKING_DEBUG_OPTIONS)
endif()
configure_file(kmm_kbanking.rc "${CMAKE_CURRENT_BINARY_DIR}/kmm_kbanking.rc" @ONLY)

########### install files ###############

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_kbanking.rc
        DESTINATION ${KXMLGUI_INSTALL_DIR}/kmm_kbanking)

install(TARGETS kmm_kbanking
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney/")

install(FILES kbanking.kcfg
        DESTINATION ${KCFG_INSTALL_DIR})
