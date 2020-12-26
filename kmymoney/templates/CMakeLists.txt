########### install files ###############

install(DIRECTORY
  C de_AT de_CH de_DE da el_GR en_GB en_US en_CA es_AR es_ES es_MX fr_CA
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
