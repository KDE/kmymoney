########### install files ###############

install(DIRECTORY
  de es fr hu it pl pt_BR ro ru sv uk
  DESTINATION ${DATA_INSTALL_DIR}/kmymoney/pics/l10n
  PATTERN ".svn" EXCLUDE
  PATTERN "*Make*" EXCLUDE
  )
