########### install files ###############

INSTALL(DIRECTORY
  C de_AT de_CH de_DE dk el_GR en_GB en_US en_CA es_AR es_ES es_MX fr_CA
  fr_CH fr_FR gl_ES hu_HU it jp nl_NL pt_BR pt_PT ro_RO ru_RU sk tr_TR
  uk_UA zh_CN zh_HK zh_TW
  DESTINATION ${DATA_INSTALL_DIR}/kmymoney/templates
  PATTERN ".svn" EXCLUDE
  PATTERN "*Make*" EXCLUDE
  )
