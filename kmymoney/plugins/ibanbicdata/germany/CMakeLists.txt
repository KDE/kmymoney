
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/germany.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/germany.desktop)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/germany.desktop DESTINATION ${SERVICES_INSTALL_DIR}/ibanbicdata/)
install(FILES bankdata.de.db DESTINATION ${KDE_INSTALL_DATADIR}/kmymoney/ibanbicdata/)
