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