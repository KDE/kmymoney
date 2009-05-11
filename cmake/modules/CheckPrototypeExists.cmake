# - Check if the prototype for a function exists.
# CHECK_PROTOTYPE_EXISTS (FUNCTION HEADER VARIABLE)
#
#  FUNCTION - the name of the function you are looking for
#  HEADER - the header(s) where the prototype should be declared
#  VARIABLE - variable to store the result
#

INCLUDE(CheckTypeSize)

MACRO(CHECK_PROTOTYPE_EXISTS _SYMBOL _HEADER _RESULT)
   SET(CMAKE_EXTRA_INCLUDE_FILES ${_HEADER})
   CHECK_TYPE_SIZE(${_SYMBOL} ${_RESULT})
   SET(CMAKE_EXTRA_INCLUDE_FILES)
ENDMACRO(CHECK_PROTOTYPE_EXISTS _SYMBOL _HEADER _RESULT)

