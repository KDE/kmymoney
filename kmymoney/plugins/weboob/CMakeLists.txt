# Use KDE compiler settings to enable C++11
# include(KDECompilerSettings NO_POLICY_SCOPE)

# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/weboob.json.in ${CMAKE_CURRENT_BINARY_DIR}/weboob.json @ONLY)

add_subdirectory( dialogs )
add_subdirectory( interface )

########### next target ###############

set(weboob_PART_SRCS
  weboob.cpp
)

add_library(weboob MODULE ${weboob_PART_SRCS})

target_link_libraries(
  weboob
  PRIVATE
  weboob_interface
  weboob_dialogs
  kmm_mymoney
  kmm_widgets
  kmm_plugin
  Qt5::Concurrent
)

########### install files ###############

install(FILES weboob.rc
        DESTINATION "${KXMLGUI_INSTALL_DIR}/weboob")

install(TARGETS weboob
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney/")

