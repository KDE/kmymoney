#
# this file contains the following macros:
# KMM_ADD_UI_FILES
# KMM_CREATE_LINKS

INCLUDE(AddFileDependencies)

#############################################################################
# Generic helper macros
#
# This section was copied from http://www.vtk.org/Wiki/CMakeUserUseLATEX
#############################################################################

# Helpful list macros.
MACRO(KMM_LIST_CONTAINS var value)
  SET(${var})
  FOREACH (value2 ${ARGN})
    IF (${value} STREQUAL ${value2})
      SET(${var} TRUE)
    ENDIF (${value} STREQUAL ${value2})
  ENDFOREACH (value2)
ENDMACRO(KMM_LIST_CONTAINS)

# Parse macro arguments.
MACRO(KMM_PARSE_ARGUMENTS prefix arg_names option_names)
  SET(DEFAULT_ARGS)
  FOREACH(arg_name ${arg_names})
    SET(${prefix}_${arg_name})
  ENDFOREACH(arg_name)
  FOREACH(option ${option_names})
    SET(${prefix}_${option})
  ENDFOREACH(option)

  SET(current_arg_name DEFAULT_ARGS)
  SET(current_arg_list)
  FOREACH(arg ${ARGN})
    KMM_LIST_CONTAINS(is_arg_name ${arg} ${arg_names})
    IF (is_arg_name)
      SET(${prefix}_${current_arg_name} ${current_arg_list})
      SET(current_arg_name ${arg})
      SET(current_arg_list)
    ELSE (is_arg_name)
      KMM_LIST_CONTAINS(is_option ${arg} ${option_names})
      IF (is_option)
        SET(${prefix}_${arg} TRUE)
      ELSE (is_option)
        SET(current_arg_list ${current_arg_list} ${arg})
      ENDIF (is_option)
    ENDIF (is_arg_name)
  ENDFOREACH(arg)
  SET(${prefix}_${current_arg_name} ${current_arg_list})
ENDMACRO(KMM_PARSE_ARGUMENTS)


#############################################################################
# Create Links
#############################################################################

GET_FILENAME_COMPONENT(KMM_MODULE_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)

IF (NOT EXISTS ${KMyMoney_BINARY_DIR}/kmymoney)
  FILE(MAKE_DIRECTORY ${KMyMoney_BINARY_DIR}/kmymoney)
ENDIF (NOT EXISTS ${KMyMoney_BINARY_DIR}/kmymoney)

MACRO(KMM_CREATE_LINKS)
  FOREACH(c_FILE ${ARGV})
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink
      ${CMAKE_CURRENT_SOURCE_DIR}/${c_FILE}
      ${KMyMoney_BINARY_DIR}/kmymoney/${c_FILE})
  ENDFOREACH (c_FILE)
ENDMACRO(KMM_CREATE_LINKS)

MACRO(KMM_CREATE_LINKS_BIN)
  FOREACH(c_FILE ${ARGV})
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink
      ${CMAKE_CURRENT_BINARY_DIR}/${c_FILE}
      ${KMyMoney_BINARY_DIR}/kmymoney/${c_FILE})
  ENDFOREACH (c_FILE)
ENDMACRO(KMM_CREATE_LINKS_BIN)

#############################################################################
# Handle UI and KCFG Files
#############################################################################

#create the implementation files from the ui files and add them to the list of sources
#usage: KMM_ADD_UI_FILES(foo_SRCS ${ui_files} DEPENDS ${dependencies})
#copied from KDE3_ADD_UI_FILES
MACRO(KMM_ADD_UI_FILES _sources )
  KMM_PARSE_ARGUMENTS(
    _this # prefix for the variable names
    "DEPENDS" # arguments
    ""        # options
    ${ARGN}
    )
  #message("You called KMM_ADD_UI with ui files \"${_this_DEFAULT_ARGS}\"
  #with DEPENDS ${_this_DEPENDS}")
  FOREACH (_current_FILE ${_this_DEFAULT_ARGS})

    GET_FILENAME_COMPONENT(_tmp_FILE ${_current_FILE} ABSOLUTE)
    GET_FILENAME_COMPONENT(_basename ${_tmp_FILE} NAME_WE)

    SET(_header ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)
    SET(_src ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)
    SET(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc.cpp)

    # setting the include path to the KMyMoney plugin lib
    SET(_kmm_plugin_path ${KMyMoney2_BINARY_DIR}/widgets)

    ADD_CUSTOM_COMMAND(OUTPUT ${_header}
      COMMAND ${QT_UIC_EXECUTABLE}
      ARGS
        -L ${KDE3_LIB_DIR}/kde3/plugins/designer -nounload
        -L ${_kmm_plugin_path} -o ${_header}
        ${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE}
      DEPENDS ${_this_DEPENDS} ${_tmp_FILE}
      )


    ADD_CUSTOM_COMMAND(OUTPUT ${_src}
      COMMAND ${CMAKE_COMMAND}
      ARGS
        -DKDE_UIC_PLUGIN_DIR:FILEPATH=${KDE3_LIB_DIR}/kde3/plugins/designer
        -DKMM_UIC_PLUGIN_DIR:FILEPATH=${_kmm_plugin_path}
        -DKDE_UIC_EXECUTABLE:FILEPATH=${QT_UIC_EXECUTABLE}
        -DKDE_UIC_FILE:FILEPATH=${_tmp_FILE}
        -DKDE_UIC_CPP_FILE:FILEPATH=${_src}
        -DKDE_UIC_H_FILE:FILEPATH=${_header}
        -P ${KMM_MODULE_DIR}/kmmuic.cmake
      DEPENDS ${_this_DEPENDS} ${_header} ${KMM_MODULE_DIR}/kmmuic.cmake
      )

    ADD_CUSTOM_COMMAND(OUTPUT ${_moc}
      COMMAND ${QT_MOC_EXECUTABLE}
      ARGS ${_header} -o ${_moc}
      DEPENDS ${_header}
      )

    SET(${_sources} ${${_sources}} ${_src} ${_moc} )

  ENDFOREACH (_current_FILE)
ENDMACRO(KMM_ADD_UI_FILES)

#create the implementation files from the ui files and add them to the list of sources
#usage: KMM_ADD_UI_FILES(foo_SRCS ${ui_files})
# copy from KDE3_ADD_UI_FILES
MACRO(KMM_CREATE_UI_HEADER_FILES _sources )
  FOREACH (_current_FILE ${ARGN})

    GET_FILENAME_COMPONENT(_absolute_current_FILE ${_current_FILE} ABSOLUTE) # absolut path to the UI File
    GET_FILENAME_COMPONENT(_basename ${_absolute_current_FILE} NAME_WE)  # only the basename without extention
    GET_FILENAME_COMPONENT(_path  ${_current_FILE} PATH)  # the path of the given file
    SET(_header ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)

    # setting the include path to the KMyMoney plugin lib
    SET(_kmm_plugin_path ${KMyMoney2_BINARY_DIR}/widgets)

    ADD_CUSTOM_COMMAND(OUTPUT ${_header}
      COMMAND ${QT_UIC_EXECUTABLE}
      ARGS -L ${KDE3_LIB_DIR}/kde3/plugins/designer -nounload
           -L ${_kmm_plugin_path} -o ${_header} ${_absolute_current_FILE}
      DEPENDS ${_absolute_current_FILE} ${_this_DEPENDS}
      )

    SET(${_sources} ${${_sources}} ${_header})

  ENDFOREACH (_current_FILE)
ENDMACRO(KMM_CREATE_UI_HEADER_FILES)

