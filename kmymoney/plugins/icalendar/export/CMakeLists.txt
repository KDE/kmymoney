# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/icalendarexporter.json.in
  ${CMAKE_CURRENT_BINARY_DIR}/icalendarexporter.json
  @ONLY
)

set(icalendarexporter_PART_SRCS
  schedulestoicalendar.cpp
  icalendarexporter.cpp
)

kconfig_add_kcfg_files(icalendarexporter_PART_SRCS pluginsettings.kcfgc)
add_library(icalendarexporter MODULE ${icalendarexporter_PART_SRCS})

target_include_directories(icalendarexporter PRIVATE ${LIBICAL_INCLUDE_DIRS})
target_link_libraries(icalendarexporter
  KF5::KIOWidgets
  kmm_mymoney
  kmm_plugin
  ${LIBICAL_LIBRARIES}
)

install(FILES icalendarexporter.rc
        DESTINATION "${KXMLGUI_INSTALL_DIR}/icalendarexporter")

install(TARGETS icalendarexporter
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney/")

# the KCM module

set(kcm_icalendarexporter_PART_SRCS
    kcm_icalendarexporter.cpp
    )

kconfig_add_kcfg_files(kcm_icalendarexporter_PART_SRCS pluginsettings.kcfgc)
ki18n_wrap_ui(kcm_icalendarexporter_PART_SRCS pluginsettingsdecl.ui)

add_library(kcm_icalendarexporter MODULE ${kcm_icalendarexporter_PART_SRCS})
kcoreaddons_desktop_to_json(kcm_icalendarexporter kcm_icalendarexporter.desktop)

target_link_libraries(kcm_icalendarexporter
  KF5::I18n
  KF5::ConfigWidgets
  KF5::KIOWidgets
  KF5::Completion
  Qt5::Widgets
  Qt5::Gui
)

install(TARGETS kcm_icalendarexporter
        DESTINATION "${KDE_INSTALL_PLUGINDIR}/kmymoney")

install(FILES kcm_icalendarexporter.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
