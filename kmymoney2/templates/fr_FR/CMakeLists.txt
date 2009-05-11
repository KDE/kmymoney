INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############

INSTALL(FILES comptes_par_default.kmt
	DESTINATION ${DATA_INSTALL_DIR}/kmymoney2/templates/fr_FR 
)



#original Makefile.am contents follow:

#templatedir=$(kde_datadir)/kmymoney2/templates/fr_FR
#template_DATA = comptes_par_default.kmt
#
#EXTRA_DIST = $(template_DATA)
