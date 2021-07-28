# Use KDE compiler settings to enable C++11
# include(KDECompilerSettings NO_POLICY_SCOPE)

# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/woob.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/woob.json @ONLY)

add_subdirectory( dialogs )
add_subdirectory( interface )

########### next target ###############

set(woob_PART_SRCS
        woob.cpp
)

qt5_add_resources(woob_PART_SRCS woob.qrc)

kcoreaddons_add_plugin(woob
                       SOURCES ${woob_PART_SRCS}
                       INSTALL_NAMESPACE "kmymoney")

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(woob
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

target_link_libraries(
  woob
  PRIVATE
  woob_interface
  woob_dialogs
  kmm_mymoney
  kmm_widgets
  kmm_plugin
  Qt5::Concurrent
)
