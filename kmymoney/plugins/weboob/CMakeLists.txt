find_package(Qt5 COMPONENTS REQUIRED Concurrent)

# Use KDE compiler settings to enable C++11
include(KDECompilerSettings NO_POLICY_SCOPE)

# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmm_weboob.json.in ${CMAKE_CURRENT_BINARY_DIR}/kmm_weboob.json @ONLY)

add_subdirectory( dialogs )

########### next target ###############

set(kmm_weboob_PART_SRCS
  weboob.cpp
  plugin.cpp
)

add_library(kmm_weboob MODULE ${kmm_weboob_PART_SRCS})

target_link_libraries(kmm_weboob
  kmm_weboob_dialogs
  kmm_mymoney
  kmm_widgets
  kmm_plugin
  KF5::KrossCore
)

########### install files ###############

install(FILES weboob.py
        DESTINATION "${DATA_INSTALL_DIR}/kmm_weboob")

install(FILES kmm_weboob.rc
        DESTINATION "${KXMLGUI_INSTALL_DIR}/kmm_weboob")

install(TARGETS kmm_weboob
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney/")

