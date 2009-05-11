INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############

INSTALL(FILES  old-default_accounts.kmt default_categories-template.kmt ScheduleC_business.kmt ScheduleE_rental-property.kmt
	DESTINATION ${DATA_INSTALL_DIR}/kmymoney2/templates/en_US  
)



#original Makefile.am contents follow:

#templatedir=$(kde_datadir)/kmymoney2/templates/en_US
#template_DATA = old-default_accounts.kmt default_categories-template.kmt ScheduleC_business.kmt ScheduleE_rental-property.kmt
#
#EXTRA_DIST = $(template_DATA)
