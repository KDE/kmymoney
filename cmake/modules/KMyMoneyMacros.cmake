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
include(ECMAddTests)
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

function(add_docbook_manual)
    # Arguments per CMake docs
    set(options INSTALL)
    set(oneValueArgs TARGET_NAME DOCBOOK_SRC SRC_DIR OUTPUT_DIR INSTALL_DIR CUSTOM_CSS COMMON_ASSETS_DIR)
    set(multiValueArgs)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Required args
    foreach(param TARGET_NAME DOCBOOK_SRC SRC_DIR OUTPUT_DIR INSTALL_DIR)
        if(NOT ARG_${param})
            message(FATAL_ERROR "add_docbook_manual: ${param} is required")
        endif()
    endforeach()

    # Asset patterns (same idea as install, excluding *.html)
    set(DOCBOOK_ASSET_PATTERNS
        "*.png" "*.jpg" "*.jpeg" "*.gif" "*.svg" "*.css" "*.js"
    )

    # Optional CSS parameter (and dependency for rebuild-on-change)
    if(ARG_CUSTOM_CSS)
        set(_css_param --stringparam html.stylesheet "${ARG_CUSTOM_CSS}")
        set(_css_dep   "${ARG_CUSTOM_CSS}")
    else()
        set(_css_param)
        set(_css_dep)
    endif()

    set(_html "${ARG_OUTPUT_DIR}/index.html")

    # ---- Collect language-specific assets (preserve tree) ----
    set(_lang_globs)
    foreach(p IN LISTS DOCBOOK_ASSET_PATTERNS)
        list(APPEND _lang_globs "${ARG_SRC_DIR}/${p}")
    endforeach()
    file(GLOB_RECURSE _assets_lang RELATIVE "${ARG_SRC_DIR}" ${_lang_globs})

    set(_copied_files)
    foreach(_rel IN LISTS _assets_lang)
        set(_src "${ARG_SRC_DIR}/${_rel}")
        set(_dst "${ARG_OUTPUT_DIR}/${_rel}")
        get_filename_component(_dir "${_dst}" DIRECTORY)
        add_custom_command(
            OUTPUT "${_dst}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${_dir}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_src}" "${_dst}"
            DEPENDS "${_src}"
            VERBATIM
        )
        list(APPEND _copied_files "${_dst}")
    endforeach()

    # ---- Collect missing assets from COMMON_ASSETS_DIR (if provided) ----
    if(ARG_COMMON_ASSETS_DIR)
        set(_common_globs)
        foreach(p IN LISTS DOCBOOK_ASSET_PATTERNS)
            list(APPEND _common_globs "${ARG_COMMON_ASSETS_DIR}/${p}")
        endforeach()
        file(GLOB_RECURSE _assets_common RELATIVE "${ARG_COMMON_ASSETS_DIR}" ${_common_globs})

        foreach(_rel IN LISTS _assets_common)
            if(NOT EXISTS "${ARG_SRC_DIR}/${_rel}")
                set(_src "${ARG_COMMON_ASSETS_DIR}/${_rel}")
                set(_dst "${ARG_OUTPUT_DIR}/${_rel}")
                get_filename_component(_dir "${_dst}" DIRECTORY)
                add_custom_command(
                    OUTPUT "${_dst}"
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${_dir}"
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_src}" "${_dst}"
                    DEPENDS "${_src}"
                    VERBATIM
                )
                list(APPEND _copied_files "${_dst}")
            endif()
        endforeach()
    endif()

    # Target to copy all assets (only if there are any)
    if(_copied_files)
        add_custom_target(copy-assets-${ARG_TARGET_NAME} DEPENDS ${_copied_files})
    endif()

    # ---- Generate the HTML output with xsltproc ----
    add_custom_command(
        OUTPUT "${_html}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${ARG_OUTPUT_DIR}"
        COMMAND ${XSLTPROC_EXECUTABLE}
                --nonet
                --param passivetex.extensions '1'
                --param generate.consistent.ids '1'
                --path ${KDOCTOOLS_CUSTOMIZATION_DIR}/dtd
                --output "${_html}"
                ${_css_param}
                --xinclude
                "${DOCBOOKXSL_DIR}/xhtml/docbook.xsl"
                "${ARG_DOCBOOK_SRC}"
        DEPENDS "${ARG_DOCBOOK_SRC}" ${_css_dep} ${_copied_files}
        WORKING_DIRECTORY "${ARG_SRC_DIR}"
        COMMENT "Generating single-page HTML manual: ${ARG_TARGET_NAME}"
        VERBATIM
    )

    # Main target (like original, built by default)
    add_custom_target(${ARG_TARGET_NAME} ALL DEPENDS "${_html}")
    if(TARGET copy-assets-${ARG_TARGET_NAME})
        add_dependencies(${ARG_TARGET_NAME} copy-assets-${ARG_TARGET_NAME})
    endif()

    # Optional installation (matches patterns incl. HTML)
    if(ARG_INSTALL)
        install(DIRECTORY "${ARG_OUTPUT_DIR}/"
            DESTINATION "${ARG_INSTALL_DIR}"
            FILES_MATCHING
                PATTERN "*.html"
                PATTERN "*.png"
                PATTERN "*.jpg"
                PATTERN "*.jpeg"
                PATTERN "*.gif"
                PATTERN "*.svg"
                PATTERN "*.css"
                PATTERN "*.js"
        )
    endif()
endfunction()
