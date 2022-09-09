# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/icalendarexporter.json.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/icalendarexporter.json
  @ONLY
)

add_library(icalendar_settings_static STATIC ${CMAKE_CURRENT_BINARY_DIR}/icalendarsettings.cpp)
target_link_libraries(icalendar_settings_static KF5::ConfigGui)
kconfig_add_kcfg_files(icalendar_settings_static icalendarsettings.kcfgc)

set(icalendarexporter_PART_SRCS
  schedulestoicalendar.cpp
  icalendarexporter.cpp
)
set_property(SOURCE schedulestoicalendar.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/icalendarsettings.h)
set_property(SOURCE icalendarexporter.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/icalendarsettings.h)

qt5_add_resources(icalendarexporter_PART_SRCS icalendarexporter.qrc)

kmymoney_add_plugin(icalendarexporter SOURCES ${icalendarexporter_PART_SRCS})

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

kmymoney_add_plugin_kcm(kcm_icalendarexporter SOURCES ${kcm_icalendarexporter_PART_SRCS})

target_link_libraries(kcm_icalendarexporter
  icalendar_settings_static
  KF5::I18n
  KF5::ConfigWidgets
  KF5::KIOWidgets
  KF5::Completion
  Qt::Widgets
  Qt::Gui
)
