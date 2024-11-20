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

#
# provide option with three states AUTO, ON, OFF
#
macro(add_auto_option _name _text _default)
    if(NOT DEFINED ${_name})
        set(${_name} ${_default} CACHE STRING "${_text}" FORCE)
    else()
        set(${_name} ${_default} CACHE STRING "${_text}")
    endif()
    set_property(CACHE ${_name} PROPERTY STRINGS AUTO ON OFF)
endmacro()

#
# Ensure that if a tristate ON/OFF/AUTO feature is set to ON,
# its requirements have been met. Fail with a fatal error if not.
#
# _name:                name of a variable ENABLE_FOO representing a tristate ON/OFF/AUTO feature
# _text:                human-readable description of the feature enabled by _name, for the
#                       error message
# CHECKS <var> [<var>]: List of variable names on which the specified option depends
#                       and which are checked for values such as 1, ON, true, and not empty.
#
# If all requirements are met, the cmake variable named ${_name}_FOUND is set.
#
macro(check_auto_option _name _text)
    set(options)
    set(oneValueArgs NAME TEXT)
    set(multiValueArgs CHECKS)
    cmake_parse_arguments(CAOARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    set(_nameval ${${_name}})
    foreach(_var ${CAOARGS_CHECKS})
        set(_varval ${${_var}})
        if(${_nameval} AND NOT ${_nameval} STREQUAL "AUTO" AND NOT ${_varval})
            message(FATAL_ERROR "${_text} requested but ${_var} not found")
        endif()
    endforeach()
    set(_varstate ${_name}_FOUND)
    set(${_varstate} ON)
    message(STATUS "Option '${_text}' enabled (${_varstate})")
    #message("debug: _name ${_name} ${_nameval}  _var ${_var} ${_varval}")
endmacro()

function(add_docbook_manual)
    set(one_value_keywords TARGET_NAME DOCBOOK_SRC SRC_DIR OUTPUT_DIR INSTALL_DIR CUSTOM_CSS)
    cmake_parse_arguments(ARG "" "${one_value_keywords}" "" ${ARGN})

    foreach(param TARGET_NAME DOCBOOK_SRC SRC_DIR OUTPUT_DIR INSTALL_DIR)
        if(NOT ARG_${param})
            message(FATAL_ERROR "add_docbook_manual: ${param} is required")
        endif()
    endforeach()

    if(ARG_CUSTOM_CSS)
        set(_css_param --stringparam html.stylesheet "${ARG_CUSTOM_CSS}")
    else()
        set(_css_param)
    endif()

    set(html_file "${ARG_OUTPUT_DIR}/index.html")
    set(stamp_file "${ARG_OUTPUT_DIR}/images.stamp")

    add_custom_command(
        OUTPUT "${stamp_file}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${ARG_OUTPUT_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${ARG_SRC_DIR}" "${ARG_OUTPUT_DIR}"
        COMMAND ${CMAKE_COMMAND} -E touch "${stamp_file}"
        DEPENDS "${ARG_SRC_DIR}"
    )

    add_custom_command(
        OUTPUT "${html_file}"
        COMMAND ${XSLTPROC_EXECUTABLE}
            --nonet
            --param passivetex.extensions '1'
            --param generate.consistent.ids '1'
            --path ${KDOCTOOLS_CUSTOMIZATION_DIR}/dtd
            --output "${html_file}"
            ${_css_param}
            --xinclude
            "${DOCBOOKXSL_DIR}/xhtml/docbook.xsl"
            "${ARG_DOCBOOK_SRC}"
        DEPENDS "${ARG_DOCBOOK_SRC}" "${stamp_file}"
        WORKING_DIRECTORY "${ARG_SRC_DIR}"
        COMMENT "Generating HTML manual: ${ARG_TARGET_NAME}"
    )

    add_custom_target(${ARG_TARGET_NAME} DEPENDS "${html_file}")

    install(DIRECTORY "${ARG_OUTPUT_DIR}/"
        DESTINATION "${ARG_INSTALL_DIR}"
        FILES_MATCHING
            PATTERN "*.html" PATTERN "*.png" PATTERN "*.jpg"
            PATTERN "*.jpeg" PATTERN "*.gif" PATTERN "*.svg"
            PATTERN "*.css" PATTERN "*.js"
    )
endfunction()
