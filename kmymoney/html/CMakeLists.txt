########### install files ###############

FILE(GLOB html_files *.html)
FILE(GLOB css_files  *.css)

INSTALL(FILES ${html_files}
	DESTINATION ${DATA_INSTALL_DIR}/kmymoney/html
)
INSTALL(FILES ${css_files}
	DESTINATION ${DATA_INSTALL_DIR}/kmymoney/html
)
INSTALL(DIRECTORY images
	DESTINATION ${DATA_INSTALL_DIR}/kmymoney/html/
	PATTERN ".svn" EXCLUDE
	PATTERN "*Make*" EXCLUDE
)

