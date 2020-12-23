# Use KDE compiler settings to enable C++11
# include(KDECompilerSettings NO_POLICY_SCOPE)

# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/weboob.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/weboob.json @ONLY)

add_subdirectory( dialogs )
add_subdirectory( interface )

########### next target ###############

set(weboob_PART_SRCS
  weboob.cpp
)

qt5_add_resources(weboob_PART_SRCS weboob.qrc)

kcoreaddons_add_plugin(weboob
                       SOURCES ${weboob_PART_SRCS}
                       JSON "${CMAKE_CURRENT_BINARY_DIR}/weboob.json"
                       INSTALL_NAMESPACE "kmymoney")

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(weboob
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

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
