INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# Macro defined in FindMEINPROC.cmake handles building and installing
ADD_DOCS(kmymoney2 en)

# Manpages we have to handle manually
SET(_in ${CMAKE_CURRENT_SOURCE_DIR}/kmymoney2.1)
SET(_out ${CMAKE_CURRENT_BINARY_DIR}/kmymoney2.1.gz)

ADD_CUSTOM_COMMAND(OUTPUT ${_out}
  COMMAND gzip
  ARGS -9 -c -N ${_in} > ${_out}
  DEPENDS ${_in})
INSTALL(FILES ${_out}
  DESTINATION ${MAN_INSTALL_DIR}/man1)
ADD_CUSTOM_TARGET(manpages ALL DEPENDS ${_out})
ADD_DEPENDENCIES(documentation manpages)
