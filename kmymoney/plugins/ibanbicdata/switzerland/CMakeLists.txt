configure_file(${CMAKE_CURRENT_SOURCE_DIR}/switzerland.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/switzerland.desktop)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/switzerland.desktop DESTINATION ${SERVICES_INSTALL_DIR}/ibanbicdata/)
install(FILES bankdata.ch.db DESTINATION ${KDE_INSTALL_DATADIR}/kmymoney/ibanbicdata/)
