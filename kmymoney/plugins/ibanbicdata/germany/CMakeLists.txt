
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/germany.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/germany.desktop)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/germany.desktop DESTINATION ${SERVICES_INSTALL_DIR}/ibanbicdata/)

# see https://www.bundesbank.de/de/aufgaben/unbarer-zahlungsverkehr/serviceangebot/bankleitzahlen/download---bankleitzahlen-602592
set(BANKDATA_URL https://www.bundesbank.de/resource/blob/602632/b6f18dadd412af2b8f47fac0cd8a8dd4/mL/blz-aktuell-txt-data.txt)

add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/bankdata.de.txt
  COMMAND wget -O ${CMAKE_CURRENT_SOURCE_DIR}/bankdata.de.txt ${BANKDATA_URL}
)

add_custom_command(OUTPUT bankdata.de.db
  COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/germany.py ${CMAKE_CURRENT_SOURCE_DIR}/bankdata.de.txt
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/bankdata.de.txt
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(ibanbicdata_de
    DEPENDS bankdata.de.db
)

add_custom_target(clean_ibanbicdata_de
  COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/bankdata.de.db
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/bankdata.de.db DESTINATION ${DATA_INSTALL_DIR}/kmymoney/ibanbicdata/)
