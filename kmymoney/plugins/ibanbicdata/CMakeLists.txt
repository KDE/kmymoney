#add_subdirectory( germany )
#add_subdirectory( switzerland )
#add_subdirectory( target2 )

# patch the version with the version defined in the build system
configure_file(ibanbicdata.desktop.in ibanbicdata.desktop)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ibanbicdata.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/ibanbicdata.json @ONLY)
#kcoreaddons_desktop_to_json(ibanbicdata "${CMAKE_CURRENT_BINARY_DIR}/ibanbicdata.desktop")

set ( IBANBICDATA_SCRS
  ibanbicdata.cpp
  bicmodel.cpp
)

kcoreaddons_add_plugin(ibanbicdata
                       SOURCES ${IBANBICDATA_SCRS}
                       JSON "${CMAKE_CURRENT_BINARY_DIR}/ibanbicdata.json"
                       INSTALL_NAMESPACE "kmymoney")

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(ibanbicdata
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

target_link_libraries(ibanbicdata
  PUBLIC
    kmm_plugin
  PRIVATE
    Qt5::Sql
    KF5::I18n
)
