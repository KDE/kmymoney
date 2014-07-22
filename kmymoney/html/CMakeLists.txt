########### install files ###############

file(GLOB html_files *.html)
file(GLOB css_files  *.css)

install(FILES ${html_files}
  DESTINATION ${DATA_INSTALL_DIR}/kmymoney/html
)
install(FILES ${css_files}
  DESTINATION ${DATA_INSTALL_DIR}/kmymoney/html
)
install(DIRECTORY images
  DESTINATION ${DATA_INSTALL_DIR}/kmymoney/html
  PATTERN ".svn" EXCLUDE
  PATTERN "*Make*" EXCLUDE
)

