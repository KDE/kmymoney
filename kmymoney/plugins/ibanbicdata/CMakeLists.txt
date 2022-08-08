#add_subdirectory( germany )
#add_subdirectory( switzerland )
#add_subdirectory( target2 )

# patch the version with the version defined in the build system
configure_file(ibanbicdata.desktop.in ibanbicdata.desktop)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ibanbicdata.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/ibanbicdata.json @ONLY)

set ( IBANBICDATA_SCRS
  ibanbicdata.cpp
  bicmodel.cpp
)

kmymoney_add_plugin(ibanbicdata SOURCES ${IBANBICDATA_SCRS})

generate_export_header(iban_bic_identifier)
target_link_libraries(ibanbicdata
  PUBLIC
    kmm_plugin
  PRIVATE
    Qt5::Sql
    KF5::I18n
)
