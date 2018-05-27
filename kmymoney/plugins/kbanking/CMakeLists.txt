# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kbanking.json.in ${CMAKE_CURRENT_BINARY_DIR}/kbanking.json @ONLY)

add_subdirectory( dialogs )
add_subdirectory( widgets )
add_subdirectory( qml )
add_subdirectory( tasksettings )

set(kbanking_PART_SRCS
    kbanking.cpp
    banking.cpp
    aqbankingkmmoperators.cpp
    gwenhywfarqtoperators.cpp
    gwenkdegui.cpp
)

kconfig_add_kcfg_files(kbanking_PART_SRCS kbankingsettings.kcfgc)

kcoreaddons_add_plugin(kbanking
                       SOURCES ${kbanking_PART_SRCS}
                       JSON "${CMAKE_CURRENT_BINARY_DIR}/kbanking.json"
                       INSTALL_NAMESPACE "kmymoney")

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(kbanking
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

target_link_libraries(kbanking
                      kbanking_dialogs
                      kbanking_widgets
                      kbanking_tasksettings
                      kmm_mymoney
                      kmm_widgets
                      kmm_plugin
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
target_compile_features(kbanking PRIVATE cxx_generalized_initializers cxx_nullptr)

# Add new menu entries if debugging is enabled
set(KMM_BANKING_DEBUG_OPTIONS)
# preprocessor definitions in case this is a debug build
string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_TOLOWER)
if("${CMAKE_BUILD_TYPE_TOLOWER}" MATCHES "debug")
  file(READ kbanking_debug.rc KMM_BANKING_DEBUG_OPTIONS)
endif()
configure_file(kbanking.rc "${CMAKE_CURRENT_BINARY_DIR}/kbanking.rc" @ONLY)

########### install files ###############

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kbanking.rc
        DESTINATION ${KXMLGUI_INSTALL_DIR}/kbanking)

install(FILES kbanking.kcfg
        DESTINATION ${KCFG_INSTALL_DIR})
