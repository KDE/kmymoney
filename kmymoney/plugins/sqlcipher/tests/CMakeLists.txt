file(GLOB tests_sources "test-*.cpp")
kmm_add_tests(${tests_sources}
  LINK_LIBRARIES
    Qt::Test
    Qt::Sql
    kmm_testutils
)

target_compile_definitions(test-qsqlcipher
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
    FILE(MAKE_DIRECTORY
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        )
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/sqldrivers
        )
  endif()
endif()

# On Windows the qsqlcipher plugin shows up in QSqlDatabase::drivers() (its JSON
# metadata is read) yet fails to load when the SQLCipher/OpenSSL runtime DLLs it
# depends on cannot be resolved - which is exactly the SQLCipher 4 failure mode.
# Print `dumpbin /dependents` for the built plugin so its dependency list is
# visible in CI.
#
# We emit it two ways on purpose:
#  1. As a POST_BUILD step on the test target. Build output is always streamed in
#     the CI log, so the listing shows up even though the default `ctest`
#     invocation (which we don't control in KDE CI) hides stdout of passing tests.
#  2. As a CTest fixture required by test-qsqlcipher, so it also runs right before
#     the test. Its output is still only shown by ctest on failure or with
#     `ctest -V` / `--test-output-size-passed`.
if(WIN32 AND MSVC AND NOT BUILD_STATIC_PLUGINS)
  find_program(DUMPBIN_EXECUTABLE dumpbin)
  if(DUMPBIN_EXECUTABLE)
    add_custom_command(TARGET test-qsqlcipher POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E echo "==== qsqlcipher.dll dependencies (dumpbin /dependents) ===="
      COMMAND ${DUMPBIN_EXECUTABLE} /dependents $<TARGET_FILE:qsqlcipher>
      COMMAND ${CMAKE_COMMAND} -E echo "==========================================================="
      VERBATIM)

    # The plugin imports sqlite3.dll. SQLCipher on Windows is built as a drop-in
    # replacement named sqlite3.dll, so the question is whether the sqlite3.dll
    # deployed next to the test executable is the SQLCipher build (exports
    # sqlite3_key) or a vanilla SQLite (no sqlite3_key) that shadows it.
    #
    # The dumpbin+findstr pipe is written to a generated .bat rather than inlined
    # as `cmd /c "..."`: the dumpbin path contains spaces (Program Files) and the
    # nested quoting does not survive CMake -> build tool -> cmd, which produced
    # "/Microsoft was unexpected at this time". A script file is quoted once by
    # cmd itself and is robust. It always exits 0 (diagnostic only).
    set(_sqlcipher_check_bat "${CMAKE_CURRENT_BINARY_DIR}/check-sqlite3-key.bat")
    file(GENERATE OUTPUT "${_sqlcipher_check_bat}" CONTENT
"@echo off\r
set DLL=%~1\r
if not exist \"%DLL%\" (\r
  echo sqlite3.dll NOT found next to test exe: \"%DLL%\"\r
  exit /b 0\r
)\r
\"${DUMPBIN_EXECUTABLE}\" /exports \"%DLL%\" | findstr /i sqlite3_key\r
if errorlevel 1 echo sqlite3_key NOT found ^(vanilla SQLite^)\r
exit /b 0\r
")

    add_custom_command(TARGET test-qsqlcipher POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E echo "==== sqlite3.dll next to test exe: SQLCipher check (sqlite3_key export) ===="
      COMMAND cmd /c "${_sqlcipher_check_bat}" "$<TARGET_FILE_DIR:test-qsqlcipher>/sqlite3.dll"
      COMMAND ${CMAKE_COMMAND} -E echo "==========================================================================="
      VERBATIM)

    add_test(
      NAME qsqlcipher-dumpbin
      COMMAND ${DUMPBIN_EXECUTABLE} /dependents $<TARGET_FILE:qsqlcipher>
    )
    set_tests_properties(qsqlcipher-dumpbin PROPERTIES
      FIXTURES_SETUP QSQLCIPHER_DEPS)
    set_tests_properties(test-qsqlcipher PROPERTIES
      FIXTURES_REQUIRED QSQLCIPHER_DEPS)
  else()
    message(WARNING
      "dumpbin not found; skipping qsqlcipher plugin dependency diagnostics")
  endif()
endif()
