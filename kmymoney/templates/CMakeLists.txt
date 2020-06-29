set (kmm_templates_SOURCES
  templateloader.cpp
  templatewriter.cpp
  kloadtemplatedlg.cpp
  ktemplateexportdlg.cpp
)

set(kmm_templates_UI
  kloadtemplatedlg.ui
  ktemplateexportdlg.ui
)

ki18n_wrap_ui(kmm_templates_SOURCES ${kmm_templates_UI} )

add_library(kmm_templates SHARED ${kmm_templates_SOURCES})

#   kmm_settings
#  KF5::KIOWidgets
#  KF5::Completion
#  KF5::Notifications
#  KF5::ItemViews
#  Qt5::Gui
#  Qt5::Core
#  Alkimia::alkimia
#  kmm_mymoney


target_link_libraries(kmm_templates PUBLIC
  KF5::TextWidgets
  KF5::I18n
  kmm_models
  kmm_widgets
)

set_target_properties(kmm_templates PROPERTIES
  VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
)

generate_export_header(kmm_templates)

install(TARGETS kmm_templates ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install(DIRECTORY
  C de_AT de_CH de_DE dk el_GR en_GB en_US en_CA es_AR es_ES es_MX fr_CA
  fr_CH fr_FR gl_ES hu_HU it ja_JP nl_NL pt_BR pt_PT ro_RO ru_RU sk tr_TR
  uk_UA zh_CN zh_HK zh_TW
  DESTINATION ${DATA_INSTALL_DIR}/kmymoney/templates
  PATTERN ".svn" EXCLUDE
  PATTERN "*Make*" EXCLUDE
  )

########### test files ###############
#
# A simple testrun could be done like follows with a simple shell command:
#
# find -name \*.kmt -print0 | xargs --null xmllint --noout --dtdvalid kmt.dtd
