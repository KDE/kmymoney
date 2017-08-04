# the KCM module
set(kcm_kmm_qif_PART_SRCS
    mymoneyqifprofileeditor.cpp
    mymoneyqifprofile.cpp
    kcm_qif.cpp
    )

ki18n_wrap_ui(kcm_kmm_qif_PART_SRCS mymoneyqifprofileeditor.ui)

add_library(kcm_kmm_qif MODULE ${kcm_kmm_qif_PART_SRCS})

if(ENABLE_QIFIMPORTER)
  kcoreaddons_desktop_to_json(kcm_kmm_qif kcm_kmm_qifimport.desktop)
endif()

if(ENABLE_QIFEXPORTER)
  kcoreaddons_desktop_to_json(kcm_kmm_qif kcm_kmm_qifexport.desktop)
endif()


target_link_libraries(kcm_kmm_qif
                      kmm_mymoney
                      KF5::I18n
                      KF5::ConfigWidgets
                      KF5::Completion
                      KF5::KIOWidgets
                      )

install(TARGETS kcm_kmm_qif
        DESTINATION ${KDE_INSTALL_PLUGINDIR})

if(ENABLE_QIFIMPORTER)
  install(FILES kcm_kmm_qifimport.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
endif()
        
if(ENABLE_QIFEXPORTER)
  install(FILES kcm_kmm_qifexport.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
endif()

