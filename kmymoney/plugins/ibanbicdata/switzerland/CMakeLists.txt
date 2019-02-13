configure_file(${CMAKE_CURRENT_SOURCE_DIR}/switzerland.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/switzerland.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/switzerland.desktop DESTINATION ${SERVICES_INSTALL_DIR}/ibanbicdata/)


# see https://www.six-group.com/interbank-clearing/de/home/bank-master-data/download-bc-bank-master.html
set(BANKDATA_URL https://www.six-group.com/interbank-clearing/dam/downloads/bc-bank-master/bcbankenstamm)

add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/bankdata.ch.txt
  COMMAND wget -O ${CMAKE_CURRENT_SOURCE_DIR}/bankdata.ch.txt ${BANKDATA_URL}
)

add_custom_command(OUTPUT bankdata.ch.db
  COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/switzerland.py ${CMAKE_CURRENT_SOURCE_DIR}/bankdata.ch.txt
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/bankdata.ch.txt
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(ibanbicdata_ch
  DEPENDS bankdata.ch.db
)

add_custom_target(clean_ibanbicdata_ch
  COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/bankdata.ch.db
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/bankdata.ch.db DESTINATION ${DATA_INSTALL_DIR}/kmymoney/ibanbicdata/)
