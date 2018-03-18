# the KCM module
set(kcm_qif_PART_SRCS
    mymoneyqifprofileeditor.cpp
    mymoneyqifprofile.cpp
    kcm_qif.cpp
    )

ki18n_wrap_ui(kcm_qif_PART_SRCS mymoneyqifprofileeditor.ui)

if(ENABLE_QIFIMPORTER)
  kcoreaddons_desktop_to_json(kcm_qif kcm_qifimporter.desktop)
endif()

if(ENABLE_QIFEXPORTER)
  kcoreaddons_desktop_to_json(kcm_qif kcm_qifexporter.desktop)
endif()

kcoreaddons_add_plugin(kcm_qif
                       SOURCES ${kcm_qif_PART_SRCS}
                       JSON "${CMAKE_CURRENT_BINARY_DIR}/kcm_qif.json"
                       INSTALL_NAMESPACE "kmymoney")

#kcoreaddons_add_plugin sets LIBRARY_OUTPUT_DIRECTORY to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${INSTALL_NAMESPACE}
set_target_properties(kcm_qif
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

target_link_libraries(kcm_qif
                      kmm_mymoney
                      KF5::I18n
                      KF5::ConfigWidgets
                      KF5::Completion
                      KF5::KIOWidgets
                      )

if(ENABLE_QIFIMPORTER)
  install(FILES kcm_qifimporter.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
endif()

if(ENABLE_QIFEXPORTER)
  install(FILES kcm_qifexporter.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
endif()

