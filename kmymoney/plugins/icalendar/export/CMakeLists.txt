# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/icalendarexporter.json.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/icalendarexporter.json
  @ONLY
)

add_library(icalendar_settings_static STATIC)
target_link_libraries(icalendar_settings_static KF5::ConfigGui)
kconfig_add_kcfg_files(icalendar_settings_static icalendarsettings.kcfgc)

set(icalendarexporter_PART_SRCS
  schedulestoicalendar.cpp
  icalendarexporter.cpp
)
set_property(SOURCE schedulestoicalendar.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/icalendarsettings.h)
set_property(SOURCE icalendarexporter.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/icalendarsettings.h)

qt5_add_resources(icalendarexporter_PART_SRCS icalendarexporter.qrc)

kcoreaddons_add_plugin(icalendarexporter
                       SOURCES ${icalendarexporter_PART_SRCS}
                       INSTALL_NAMESPACE "kmymoney")
#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(icalendarexporter
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

target_include_directories(icalendarexporter PRIVATE ${LibIcal_INCLUDE_DIRS})
target_link_libraries(icalendarexporter
  icalendar_settings_static
  KF5::KIOWidgets
  kmm_mymoney
  kmm_plugin
  ${LibIcal_LIBRARIES}
)

# the KCM module

set(kcm_icalendarexporter_PART_SRCS
    kcm_icalendarexporter.cpp
    )
set_property(SOURCE kcm_icalendarexporter.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/icalendarsettings.h)

ki18n_wrap_ui(kcm_icalendarexporter_PART_SRCS icalendarsettingsdecl.ui)

kcoreaddons_add_plugin(kcm_icalendarexporter
                       SOURCES ${kcm_icalendarexporter_PART_SRCS}
                       INSTALL_NAMESPACE "kmymoney")

kcoreaddons_desktop_to_json(kcm_icalendarexporter kcm_icalendarexporter.desktop)

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(kcm_icalendarexporter
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

target_link_libraries(kcm_icalendarexporter
  icalendar_settings_static
  KF5::I18n
  KF5::ConfigWidgets
  KF5::KIOWidgets
  KF5::Completion
  Qt5::Widgets
  Qt5::Gui
)

install(FILES kcm_icalendarexporter.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
