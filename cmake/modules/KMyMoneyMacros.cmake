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



find_program(XSLTPROC_EXECUTABLE xsltproc)
if(XSLTPROC_EXECUTABLE)
    message(STATUS "${XSLTPROC_EXECUTABLE} found")
endif()

#
# docbook based documentation
#
find_package(DocBookXSL MODULE)

if(DOCBOOKXSL_DIR AND XSLTPROC_EXECUTABLE)
    # always check doc target
    add_custom_target(doc ALL)
    option(ENABLE_DOCBOOK_DOCS "build docbook documentation" ON)
    set(DOCBOOK_DOCS_ENABLED 1)
    message(STATUS "xsltproc docbook generator found")
    add_custom_target(docbook-doc)
    add_dependencies(doc docbook-doc)
endif()

#
# generate output file from docbook xml source template or file
#
# @param _target            base name for the generated file
# @param CSS_FILE <file>    add file with css styles to the generated output
# @param DEPENDS <list>     list of files the generated output depends on
# @param LOAD_TRACE         show load traces of external entities)
# @param SOURCE <file>      alternative docbook xml file to generated the output from
#                           (without variable substitution)
# @param FORMATS <formats>  list with output formats to generate ('html' and/or 'man')
# @param INSTALL_DIR <dir>  absolute file path to install generated files
# @param INSTALL_SUBDIR <dir>  relative file path to install generated files based on
#                           default installation directory
# @param MAN_CATEGORY <cat> category for creating man pages (also used for html output)
# @param TEMPLATE <file>    docbook xml template file to generated the output from
#                           (with '@var@' variable substitution)
#
function(add_docbook _target)
    set(options LOAD_TRACE)
    set(oneValueArgs BASENAME CSS_FILE INSTALL_DIR INSTALL_SUBDIR MAN_CATEGORY OUTPUT_DIR SOURCE TEMPLATE TARGET_NAME)
    set(multiValueArgs FORMATS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    message(STATUS "add_docbook ${ARGN}")

    foreach(_format ${ARG_FORMATS})
        if(ARG_TEMPLATE)
            set(_xmlfile "${CMAKE_CURRENT_BINARY_DIR}/${_target}-${_format}.xml")
            get_filename_component(_infile ${ARG_TEMPLATE} ABSOLUTE)
            configure_file(${_infile} ${_xmlfile})
        else()
            get_filename_component(_infile ${ARG_SOURCE} ABSOLUTE)
            set(_xmlfile ${_infile})
        endif()
        if(${_format} STREQUAL "man")
            set(_outname "${_target}.${ARG_MAN_CATEGORY}")
            set(STYLESHEET "${DOCBOOKXSL_DIR}/manpages/docbook.xsl")
            if(ARG_INSTALL_DIR)
                set(INSTALL_DIR ${ARG_INSTALL_DIR})
            else()
                set(INSTALL_DIR ${CMAKE_INSTALL_DATADIR}/man/man${ARG_MAN_CATEGORY})
            endif()
        else()
            if(ARG_BASENAME)
                set(_outname "${ARG_BASENAME}.html")
            elseif(NOT ARG_MAN_CATEGORY)
                set(_outname "${_target}.html")
            else()
                set(_outname "${_target}.${ARGS_MAN_CATEGORY}.html")
            endif()
            set(STYLESHEET "${DOCBOOKXSL_DIR}/html/docbook.xsl")
            if(ARG_INSTALL_DIR)
                set(INSTALL_DIR ${ARG_INSTALL_DIR})
            else()
                set(INSTALL_DIR ${CMAKE_INSTALL_DATADIR}/doc/${ARG_INSTALL_SUBDIR})
            endif()
        endif()
        if(ARG_OUTPUT_DIR)
            set(OUTPUT_DIR "${ARG_OUTPUT_DIR}")
        else()
            set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
        endif()
        set(_outfile ${OUTPUT_DIR}/${_outname})
        if(KDOCTOOLS_CUSTOMIZATION_DIR)
            list(APPEND optional --path ${KDOCTOOLS_CUSTOMIZATION_DIR}/dtd)
        endif()
        if(ARG_LOAD_TRACE)
            list(APPEND optional --load-trace)
        endif()
        if(ARG_CSS_FILE)
            list(APPEND optional --stringparam html.stylesheet ${ARG_CSS_FILE})
        endif()

        list(APPEND optional --stringparam textdata.default.encoding UTF-8)

        message(STATUS "${XSLTPROC_EXECUTABLE} --output ${_outfile} --nonet --xinclude --param passivetex.extensions '1' --param generate.consistent.ids '1' ${optional} ${STYLESHEET} ${_xmlfile}")
        add_custom_command(
            OUTPUT ${_outfile}
            COMMAND ${XSLTPROC_EXECUTABLE} --output ${_outfile} --nonet --xinclude --param passivetex.extensions '1' --param generate.consistent.ids '1' ${optional} ${STYLESHEET} ${_xmlfile}
            DEPENDS ${XSLTPROC_EXECUTABLE} ${_xmlfile} ${ARG_DEPENDS}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
        if(ARG_TARGET_NAME)
            set(TARGET_NAME ${ARG_TARGET_NAME})
        else()
            set(TARGET_NAME docbook-doc-${_outname})
        endif()
        add_custom_target(${TARGET_NAME} DEPENDS ${_outfile})
        add_dependencies(docbook-doc ${TARGET_NAME})
        install(FILES ${_outfile} DESTINATION ${INSTALL_DIR})
    endforeach()
endfunction()

#set(CONFIG_VERBOSE 1)

### copy tests to builddir so that generated tests and static tests
### are all in one place.
### todo how to add more filetypes
macro(COPYDIR)
    set(options)
    set(oneValueArgs OUTPUT_DIR RETURN_SOURCE_NAMES)
    set(multiValueArgs SOURCES TYPES)
    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    message(STATUS "${ARGN}")
    if(ARGS_OUTPUT_DIR)
        set(OUTPUT_DIR ${ARGS_OUTPUT_DIR})
    else()
        set(OUTPUT_DIR ${PROJECT_BINARY_DIR}/${DIR})
    endif()
    foreach(FILE_TYPE ${ARGS_TYPES})
        foreach(DIR ${ARGS_SOURCES})
            file(GLOB FILES "${CMAKE_SOURCE_DIR}/${DIR}/${FILE_TYPE}" )
            file(MAKE_DIRECTORY ${OUTPUT_DIR})
            foreach(FILE ${FILES})
                get_filename_component(FILENAME ${FILE} NAME)
                set(TARGET ${OUTPUT_DIR}/${FILENAME})
                message(STATUS "copying ${FILE} to ${TARGET}")
                configure_file(${FILE} ${TARGET} COPYONLY)
                if(CONFIG_VERBOSE)
                    message("FROM: ${FILE}\nTO: ${TARGET}\n")
                endif()
                if(ARGS_RETURN_SOURCE_NAMES)
                    list(APPEND ${ARGS_RETURN_SOURCE_NAMES} ${file})
                endif()
            endforeach()
        endforeach()
    endforeach()
endmacro()

function(kmm_kdoctools_install podir)
    file(GLOB lang_dirs "${podir}/*")
    if (NOT KDE_INSTALL_DOCBUNDLEDIR)
        if (HTML_INSTALL_DIR) # TODO KF6: deprecated, remove
            set(KDE_INSTALL_DOCBUNDLEDIR ${HTML_INSTALL_DIR})
        else()
            set(KDE_INSTALL_DOCBUNDLEDIR share/doc/HTML)
        endif()
    endif()
    foreach(lang_dir ${lang_dirs})
        get_filename_component(lang ${lang_dir} NAME)
        file(GLOB_RECURSE docbooks RELATIVE "${lang_dir}" "${lang_dir}/docs/*.docbook")
        foreach(docbook ${docbooks})
            string(REGEX MATCH "^docs/(.*)/index.docbook" match ${docbook})
            if (match)
                string(REPLACE ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} OUTPUT_DIR ${lang_dir}/docs)
                message(STATUS "${lang_dir}/${match} ${OUTPUT_DIR}")
                COPYDIR(
                    SOURCES ${lang_dir}/kmymoney
                    TYPES *.png *.svg *.css
                    OUTPUT_DIR ${OUTPUT_DIR}/kmymoney
                )
                add_docbook(
                    index
                    TARGET_NAME po-${lang}-docs-kmymoney-index-html
                    SOURCE ${lang_dir}/${match}
                    OUTPUT_DIR ${OUTPUT_DIR}/kmymoney
                    FORMATS html
                    CSS_FILE ${CMAKE_SOURCE_DIR}/doc/kmymoney.css
                    INSTALL_DIR ${KDE_INSTALL_DOCBUNDLEDIR}/${lang}
                )
            endif()
        endforeach()
    endforeach()
endfunction()
