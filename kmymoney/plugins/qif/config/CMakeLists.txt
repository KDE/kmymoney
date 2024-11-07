# the KCM module
set(kcm_qif_PART_SRCS
    mymoneyqifprofileeditor.cpp
    kcm_qif.cpp
    )

ki18n_wrap_ui(kcm_qif_PART_SRCS mymoneyqifprofileeditor.ui)

kmymoney_add_plugin_kcm(kcm_qif SOURCES ${kcm_qif_PART_SRCS})
target_link_libraries(kcm_qif
                      kmm_mymoney
                      KF${QT_MAJOR_VERSION}::I18n
                      KF${QT_MAJOR_VERSION}::ConfigWidgets
                      KF${QT_MAJOR_VERSION}::Completion
                      KF${QT_MAJOR_VERSION}::KIOWidgets
                      kmm_kcmodule
                      kmm_yesno
                      )
