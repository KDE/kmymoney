# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/checkprinting.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/checkprinting.json @ONLY)

add_library(checkprinting_settings_static STATIC ${CMAKE_CURRENT_BINARY_DIR}/checkprintingsettings.cpp)
target_link_libraries(checkprinting_settings_static KF${QT_MAJOR_VERSION}::ConfigGui)
kconfig_add_kcfg_files(checkprinting_settings_static checkprintingsettings.kcfgc)

set(checkprinting_PART_SRCS
    numbertowords.cpp
    checkprinting.cpp
    )
set_property(SOURCE checkprinting.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/checkprintingsettings.h)

qt_add_resources(checkprinting_PART_SRCS checkprinting.qrc)

kmymoney_add_plugin(checkprinting SOURCES ${checkprinting_PART_SRCS})

target_link_libraries(checkprinting
  Qt::PrintSupport
  KF${QT_MAJOR_VERSION}::I18n
  checkprinting_settings_static
  kmm_mymoney
  kmm_plugin
  kmm_printer
  kmm_selections
)

# the KCM module

set(kcm_checkprinting_PART_SRCS
    kcm_checkprinting.cpp
    )
set_property(SOURCE kcm_checkprinting.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/checkprintingsettings.h)

ki18n_wrap_ui(kcm_checkprinting_PART_SRCS checkprintingsettingsdecl.ui)

kmymoney_add_plugin_kcm(kcm_checkprinting SOURCES ${kcm_checkprinting_PART_SRCS})

target_link_libraries(
                      kcm_checkprinting
                      checkprinting_settings_static
                      kmm_url
                      kmm_kcmodule
                      Qt::PrintSupport
                      KF${QT_MAJOR_VERSION}::I18n
                      KF${QT_MAJOR_VERSION}::Completion
                      KF${QT_MAJOR_VERSION}::KIOWidgets
                      KF${QT_MAJOR_VERSION}::CoreAddons
                      )

file(GLOB TEMPLATE_FILES
    templates/*.html
)

install(FILES ${TEMPLATE_FILES} 
  DESTINATION ${KDE_INSTALL_DATADIR}/checkprinting)
