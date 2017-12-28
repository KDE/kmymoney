# the KCM module
set(kcm_qif_PART_SRCS
    mymoneyqifprofileeditor.cpp
    mymoneyqifprofile.cpp
    kcm_qif.cpp
    )

ki18n_wrap_ui(kcm_qif_PART_SRCS mymoneyqifprofileeditor.ui)

add_library(kcm_qif MODULE ${kcm_qif_PART_SRCS})

if(ENABLE_QIFIMPORTER)
  kcoreaddons_desktop_to_json(kcm_qif kcm_qifimporter.desktop)
endif()

if(ENABLE_QIFEXPORTER)
  kcoreaddons_desktop_to_json(kcm_qif kcm_qifexporter.desktop)
endif()


target_link_libraries(kcm_qif
                      kmm_mymoney
                      KF5::I18n
                      KF5::ConfigWidgets
                      KF5::Completion
                      KF5::KIOWidgets
                      )

install(TARGETS kcm_qif
        DESTINATION ${KDE_INSTALL_PLUGINDIR})

if(ENABLE_QIFIMPORTER)
  install(FILES kcm_qifimporter.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
endif()
        
if(ENABLE_QIFEXPORTER)
  install(FILES kcm_qifexporter.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
endif()

