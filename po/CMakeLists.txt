INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

FILE(GLOB PO_FILES *.po)
ADD_TRANSLATIONS(kmymoney2 ${PO_FILES})


add_custom_target(message-stats
  COMMAND "sh" "${CMAKE_CURRENT_SOURCE_DIR}/message-stats.sh" "${CMAKE_CURRENT_SOURCE_DIR}")

########### install files ###############
