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

