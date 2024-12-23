
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/target2.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/target2.desktop)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/target2.desktop DESTINATION ${SERVICES_INSTALL_DIR}/ibanbicdata/)
install(FILES bankdata.target2.db DESTINATION ${KDE_INSTALL_DATADIR}/kmymoney/ibanbicdata/)
