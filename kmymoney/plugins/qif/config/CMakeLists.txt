# the KCM module
set(kcm_qif_PART_SRCS
    mymoneyqifprofileeditor.cpp
    kcm_qif.cpp
    )

ki18n_wrap_ui(kcm_qif_PART_SRCS mymoneyqifprofileeditor.ui)

if(ENABLE_QIFIMPORTER OR ENABLE_QIFEXPORTER)
  kmymoney_add_plugin_kcm(kcm_qif SOURCES ${kcm_qif_PART_SRCS})
  target_link_libraries(kcm_qif
                        kmm_mymoney
                        KF5::I18n
                        KF5::ConfigWidgets
                        KF5::Completion
                        KF5::KIOWidgets
                        )

endif()

