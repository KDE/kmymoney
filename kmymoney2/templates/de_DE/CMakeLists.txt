INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############

INSTALL(FILES default_accounts.kmt
	DESTINATION ${DATA_INSTALL_DIR}/kmymoney2/templates/de_DE 
)



#original Makefile.am contents follow:

#templatedir=$(kde_datadir)/kmymoney2/templates/de_DE
#template_DATA = default_accounts.kmt
#
#EXTRA_DIST = $(template_DATA)
