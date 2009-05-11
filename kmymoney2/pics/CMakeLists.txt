INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############

INSTALL(FILES  startlogo.png  titlelabel_background.png 
	DESTINATION ${DATA_INSTALL_DIR}/kmymoney2/pics/ 
)



#original Makefile.am contents follow:

#kmymoneydir = $(kde_datadir)/kmymoney2/pics/
#kmymoney_DATA = startlogo.png  titlelabel_background.png 
#
