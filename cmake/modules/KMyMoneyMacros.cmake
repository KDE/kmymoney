# SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@kde.org>
# Redistribution and use is allowed according to the terms of the BSD license.

#
# this file contains the following macros:
# KMM_CREATE_LINKS
# KMM_CREATE_LINKS_BIN
# KMYMONEY_ADD_PLUGIN

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

if (TRUE)
    function(kmymoney_add_plugin plugin)
        message(TEST)
        set(options)
        set(oneValueArgs JSON)
        set(multiValueArgs SOURCES)
        cmake_parse_arguments(KC_ADD_PLUGIN "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

        if(NOT KC_ADD_PLUGIN_SOURCES)
            message(FATAL_ERROR "kmymoney_add_plugin called without SOURCES parameter")
        endif()
        get_filename_component(json "${KC_ADD_PLUGIN_JSON}" REALPATH)

        add_library(${plugin} STATIC ${KC_ADD_PLUGIN_SOURCES})
        set_property(TARGET ${plugin} APPEND PROPERTY AUTOGEN_TARGET_DEPENDS ${json})
        set_property(TARGET ${plugin} APPEND PROPERTY COMPILE_DEFINITIONS QT_STATICPLUGIN)
    endfunction()
else()
    function(kmymoney_add_plugin)
        kcoreaddons_add_plugin(${ARGN} INSTALL_NAMESPACE kmymoney)
    endfunction()
endif()