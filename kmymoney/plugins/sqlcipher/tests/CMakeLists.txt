include(ECMAddTests)

file(GLOB tests_sources "*-test.cpp")
ecm_add_tests(${tests_sources}
  LINK_LIBRARIES
    Qt5::Test
    Qt5::Sql
)

target_compile_definitions(qsqlcipher-test
  PRIVATE
  QSQLCIPHERPATH="${CMAKE_INSTALL_PREFIX}/${PLUGIN_INSTALL_DIR}")

# the test application is stored in CMAKE_RUNTIME_OUTPUT_DIRECTORY and
# the plugin is stored in CMAKE_LIBRARY_OUTPUT_DIRECTORY. On FreeBSD
# and Linux systems Qt expects the plugin to be found in various
# specific locations, where one is a sub-directory called sqldrivers
# in CMAKE_RUNTIME_OUTPUT_DIRECTORY. To achieve a running test, we
# simply create a symlink to CMAKE_LIBRARY_OUTPUT_DIRECTORY.

# Windows and MacOS may also need some adjustments, but I leave that
# to a developer under these system to make additions here
if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR CMAKE_SYSTEM_NAME MATCHES "Linux")
  if (NOT EXISTS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/sqldrivers")
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/sqldrivers
        )
  endif()
endif()
