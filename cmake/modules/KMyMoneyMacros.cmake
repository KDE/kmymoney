#
# this file contains the following macros:
# KMM_CREATE_LINKS
# KMM_CREATE_LINKS_BIN

#############################################################################
# Create Links
#############################################################################

IF (NOT EXISTS ${KMyMoney_BINARY_DIR}/kmymoney)
  FILE(MAKE_DIRECTORY ${KMyMoney_BINARY_DIR}/kmymoney)
ENDIF (NOT EXISTS ${KMyMoney_BINARY_DIR}/kmymoney)

MACRO(KMM_CREATE_LINKS)
  FOREACH(c_FILE ${ARGV})
    IF(WIN32)
      CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${c_FILE}
        ${KMyMoney_BINARY_DIR}/kmymoney/${c_FILE}
        COPYONLY)
    ELSE(WIN32)
      EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_CURRENT_SOURCE_DIR}/${c_FILE}
        ${KMyMoney_BINARY_DIR}/kmymoney/${c_FILE})
    ENDIF(WIN32)
  ENDFOREACH (c_FILE)
ENDMACRO(KMM_CREATE_LINKS)


function(kmymoney_add_plugin name)
    if (BUILD_STATIC_PLUGINS)
        kcoreaddons_add_plugin(${name} ${ARGN} STATIC INSTALL_NAMESPACE "kmymoney_plugins")
    else()
        kcoreaddons_add_plugin(${name} ${ARGN} INSTALL_NAMESPACE "kmymoney_plugins")
    endif()
endfunction()

function(kmymoney_add_plugin_kcm name)
    if (BUILD_STATIC_PLUGINS)
        kcoreaddons_add_plugin(${name} ${ARGN} STATIC INSTALL_NAMESPACE "kmymoney_plugins/kcms")
    else()
        kcoreaddons_add_plugin(${name} ${ARGN} INSTALL_NAMESPACE "kmymoney_plugins/kcms")
    endif()
endfunction()

# add kmymoney specific unit tests
#
# It supports the same parameter add add_ecm_tests()
#
# supported global variables:
# - KMM_ADD_TESTS_ENVIRONMENT   add environment settings to all tests defined with kmm_add_tests()
#
function(kmm_add_tests)
  set(options GUI)
  set(oneValueArgs NAME_PREFIX TARGET_NAMES_VAR TEST_NAMES_VAR WORKING_DIRECTORY)
  set(multiValueArgs COMPILE_DEFINITIONS ENVIRONMENT LINK_LIBRARIES)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  set(test_args)
  if(KMM_ADD_TESTS_ENVIRONMENT)
    list(APPEND ARG_ENVIRONMENT ${KMM_ADD_TESTS_ENVIRONMENT})
  endif()
  if(DEFINED ARG_WORKING_DIRECTORY)
    list(APPEND test_args WORKING_DIRECTORY ${ARG_WORKING_DIRECTORY})
  endif()
  if(DEFINED ARG_COMPILE_DEFINITIONS)
      list(APPEND test_args COMPILE_DEFINITIONS ${ARG_COMPILE_DEFINITIONS})
  endif()
  if(DEFINED ARG_ENVIRONMENT)
      list(APPEND test_args ENVIRONMENT ${ARG_ENVIRONMENT})
  endif()
  if(DEFINED ARG_GUI)
      list(APPEND test_args GUI)
  endif()
  if(DEFINED ARG_LINK_LIBRARIES)
      list(APPEND test_args LINK_LIBRARIES ${ARG_LINK_LIBRARIES})
  endif()
  if(DEFINED ARG_NAME_PREFIX)
      list(APPEND test_args NAME_PREFIX ${ARG_NAME_PREFIX})
  endif()
  if(DEFINED ARG_TARGET_NAME_VAR)
      list(APPEND test_args TARGET_NAME_VAR ${ARG_TARGET_NAME_VAR})
  endif()
  if(DEFINED ARG_TEST_NAME_VAR)
      list(APPEND test_args TEST_NAME_VAR ${ARG_TEST_NAME_VAR})
  endif()
  ecm_add_tests(
    ${ARG_UNPARSED_ARGUMENTS}
    ${test_args}
  )
endfunction()
