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

qt_add_resources(woob_PART_SRCS woob.qrc)

kmymoney_add_plugin(woob SOURCES ${woob_PART_SRCS})

target_link_libraries(
  woob
  PRIVATE
  woob_interface
  woob_dialogs
  kmm_mymoney
  kmm_widgets
  kmm_plugin
  Qt::Concurrent
)
