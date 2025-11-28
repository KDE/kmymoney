# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kbanking.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/kbanking.json @ONLY)

file(STRINGS "${AQBANKING_INCLUDE_DIRS}/aqbanking/banking.h" HAVE_REGISTRATION_FEATURE REGEX "AB_Banking_RuntimeConfig_SetCharValue")
if (NOT ${HAVE_REGISTRATION_FEATURE} STREQUAL "")
  add_definitions(-DENABLE_FINTS_REGISTRATION)
endif()

if ("${gwenhywfar_VERSION}" VERSION_LESS "5.11.0")
  add_definitions(-UQT_DISABLE_DEPRECATED_BEFORE)
endif()

add_subdirectory( dialogs )
add_subdirectory( widgets )
add_subdirectory( tasksettings )

set(kbanking_PART_SRCS
    kbanking.cpp
    banking.cpp
    aqbankingkmmoperators.cpp
    gwenhywfarqtoperators.cpp
    gwenkdegui.cpp
)

# Add new menu entries if debugging is enabled
set(KMM_BANKING_DEBUG_OPTIONS)
# preprocessor definitions in case this is a debug build
string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_TOLOWER)
if("${CMAKE_BUILD_TYPE_TOLOWER}" MATCHES "debug")
    file(READ kbanking_debug_rc KMM_BANKING_DEBUG_OPTIONS)
endif()
configure_file(kbanking.rc.in "${CMAKE_CURRENT_BINARY_DIR}/kbanking.rc" @ONLY)

# need to load kbanking.qrc from ${CMAKE_CURRENT_BINARY_DIR} because the resource files
# must be located in the same directory as the .qrc file, or one of its subdirectories.
configure_file(kbanking.qrc "${CMAKE_CURRENT_BINARY_DIR}/kbanking.qrc" COPYONLY)
qt_add_resources(kbanking_PART_SRCS "${CMAKE_CURRENT_BINARY_DIR}/kbanking.qrc")

qt_add_resources(kbanking_PART_SRCS "qml/chipTan/chipTan.qrc")

kmymoney_add_plugin(kbanking SOURCES ${kbanking_PART_SRCS})

target_link_libraries(kbanking
                      kbanking_dialogs
                      kbanking_widgets
                      kbanking_tasksettings
                      kmm_mymoney
                      kmm_base_widgets
                      kmm_plugin
                      KF${QT_MAJOR_VERSION}::ConfigCore
                      KF${QT_MAJOR_VERSION}::ConfigGui
                      Qt::Core
                      Alkimia::alkimia
                      gwenhywfar::core
                      gwenhywfar::gui-cpp
                      gwenhywfar::gui-qt${QT_MAJOR_VERSION}
                      aqbanking::aqbanking
)

########### install files ###############

install(FILES kbanking.kcfg
        DESTINATION ${KDE_INSTALL_KCFGDIR})
