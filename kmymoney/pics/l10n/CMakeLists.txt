########### install files ###############

INSTALL(DIRECTORY
  es
  DESTINATION ${DATA_INSTALL_DIR}/kmymoney/pics/l10n
  PATTERN ".svn" EXCLUDE
  PATTERN "*Make*" EXCLUDE
  )
