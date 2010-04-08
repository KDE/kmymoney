# Find Gwenhywfar
#
#  GWENHYWFAR_FOUND - system has Gwenhywfar with the minimum version needed
#  GWENHYWFAR_INCLUDE_DIRS - the Gwenhywfar include directories
#  GWENHYWFAR_LIBRARIES - The libraries needed to use Gwenhywfar
#  GWENHYWFAR_VERSION = The version of Gwenhywfar as defined in version.h

include(MacroEnsureVersion)

set(GWENHYWFAR_FOUND FALSE)

if(NOT GWENHYWFAR_MIN_VERSION)
  set(GWENHYWFAR_MIN_VERSION "3.10.1")
endif(NOT GWENHYWFAR_MIN_VERSION)

if (WIN32)
  file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" _program_FILES_DIR)
endif(WIN32)

set(GWENHYWFAR_FIND_REQUIRED ${Gwenhywfar_FIND_REQUIRED})
if(GWENHYWFAR_INCLUDE_DIRS AND GWENHYWFAR_LIBRARIES)

  # Already in cache, be silent
  set(GWENHYWFAR_FIND_QUIETLY TRUE)

endif(GWENHYWFAR_INCLUDE_DIRS AND GWENHYWFAR_LIBRARIES)

#set the root from the GWENHYWFAR_BASE environment
file(TO_CMAKE_PATH "$ENV{GWENHYWFAR_BASE}" gwenhywfar_root )
#override the root from GWENHYWFAR_BASE defined to cmake
if(DEFINED GWENHYWFAR_BASE)
  file(TO_CMAKE_PATH "${GWENHYWFAR_BASE}" gwenhywfar_root )
endif(DEFINED GWENHYWFAR_BASE)

find_path(GWENHYWFAR_INCLUDE_DIR NAMES gwenhywfar/version.h
  HINTS /usr/include/gwenhywfar3 /usr/local/include/gwenhywfar3 ${gwenhywfar_root}/include/gwenhywfar3 ${_program_FILES_DIR}/gwenhywfar/include
)

find_library(GWENHYWFAR_LIBRARY NAMES gwenhywfar libgwenhywfar
  HINTS ${gwenhywfar_root}/lib ${_program_FILES_DIR}/gwenhywfar/lib
)

find_library(GWENHYWFAR_GUI_LIBRARY NAMES gwengui-qt4 libgwengui-qt4
  HINTS ${gwenhywfar_root}/lib ${_program_FILES_DIR}/gwenhywfar/lib
)

if(GWENHYWFAR_GUI_LIBRARY STREQUAL "GWENHYWFAR_GUI_LIBRARY-NOTFOUND")
  set(GWENHYWFAR_GUI_LIBRARY "")
endif(GWENHYWFAR_GUI_LIBRARY STREQUAL "GWENHYWFAR_GUI_LIBRARY-NOTFOUND")

set(GWENHYWFAR_INCLUDE_DIRS ${GWENHYWFAR_INCLUDE_DIR})
set(GWENHYWFAR_LIBRARIES ${GWENHYWFAR_LIBRARY} ${GWENHYWFAR_GUI_LIBRARY})

if(GWENHYWFAR_INCLUDE_DIRS AND GWENHYWFAR_LIBRARIES)
  set(FIND_GWENHYWFAR_VERSION_SOURCE
    "#include <stdio.h>\n #include <gwenhywfar/version.h>\n int main()\n {\n printf(\"%s\",GWENHYWFAR_VERSION_STRING);return 1;\n }\n")
  set(FIND_GWENHYWFAR_VERSION_SOURCE_FILE ${CMAKE_BINARY_DIR}/CMakeTmp/FindGWENHYWFAR.cxx)
  file(WRITE "${FIND_GWENHYWFAR_VERSION_SOURCE_FILE}" "${FIND_GWENHYWFAR_VERSION_SOURCE}")

  set(FIND_GWENHYWFAR_VERSION_ADD_INCLUDES
    "-DINCLUDE_DIRECTORIES:STRING=${GWENHYWFAR_INCLUDE_DIRS}")

  try_run(RUN_RESULT COMPILE_RESULT
    ${CMAKE_BINARY_DIR}
    ${FIND_GWENHYWFAR_VERSION_SOURCE_FILE}
    CMAKE_FLAGS "${FIND_GWENHYWFAR_VERSION_ADD_INCLUDES}"
    RUN_OUTPUT_VARIABLE GWENHYWFAR_VERSION)

  if(COMPILE_RESULT AND RUN_RESULT EQUAL 1)
    message(STATUS "Found Gwenhywfar version ${GWENHYWFAR_VERSION}")
    macro_ensure_version2(${GWENHYWFAR_MIN_VERSION} ${GWENHYWFAR_VERSION} GWENHYWFAR_VERSION_OK)
    if(NOT GWENHYWFAR_VERSION_OK)
      message(STATUS "Gwenhywfar version ${GWENHYWFAR_VERSION} is too old. At least version ${GWENHYWFAR_MIN_VERSION} is needed.")
      set(GWENHYWFAR_INCLUDE_DIRS "")
      set(GWENHYWFAR_LIBRARIES "")
    else(NOT GWENHYWFAR_VERSION_OK)
      set(GWENHYWFAR_FOUND TRUE)
    endif(NOT GWENHYWFAR_VERSION_OK)
  else(COMPILE_RESULT AND RUN_RESULT EQUAL 1)
    message(FATAL_ERROR "Unable to compile or run the Gwenhywfar version detection program.")
  endif(COMPILE_RESULT AND RUN_RESULT EQUAL 1)
else(GWENHYWFAR_INCLUDE_DIRS AND GWENHYWFAR_LIBRARIES)
  message(STATUS "Could not find Gwenhywfar headers and the KBanking plugin will not be compiled. If you do have the Gwenhywfar development package installed use the GWENHYWFAR_BASE cmake variable to point to the location where Gwenhywfar is installed")
endif(GWENHYWFAR_INCLUDE_DIRS AND GWENHYWFAR_LIBRARIES)

mark_as_advanced(GWENHYWFAR_INCLUDE_DIRS GWENHYWFAR_LIBRARIES)
