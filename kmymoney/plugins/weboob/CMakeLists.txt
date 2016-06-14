find_package(Qt5 COMPONENTS REQUIRED Concurrent)
# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmm_weboob.desktop ${CMAKE_CURRENT_BINARY_DIR}/kmm_weboob.desktop)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
                    ${CMAKE_CURRENT_BINARY_DIR}/dialogs)

add_subdirectory( dialogs )

########### next target ###############

set(kmm_weboob_PART_SRCS
  weboob.cpp
  plugin.cpp
)

add_library(kmm_weboob MODULE ${kmm_weboob_PART_SRCS})
kcoreaddons_desktop_to_json(kmm_weboob kmm_weboob.desktop)

target_link_libraries(kmm_weboob
                      kmm_weboob_dialogs
                      kmm_mymoney
                      kmm_widgets
                      kmm_plugin
                      KF5::KDELibs4Support
                      KF5::KrossCore
                      Alkimia::alkimia
                      )

# Needs this C++ features: TODO: CMake 3
# Public: unique_ptr

########### install files ###############

install(FILES weboob.py
        DESTINATION  ${DATA_INSTALL_DIR}/kmm_weboob)

install(FILES kmm_weboob.rc
        DESTINATION ${KXMLGUI_INSTALL_DIR}/kmm_weboob)

install(TARGETS kmm_weboob
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_weboob.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
