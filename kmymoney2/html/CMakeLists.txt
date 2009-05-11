#ADD_SUBDIRECTORY( images ) 

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############

FILE(GLOB html_files *.html)
FILE(GLOB css_files  *.css)

INSTALL(FILES ${html_files}
	DESTINATION ${DATA_INSTALL_DIR}/kmymoney2/html
)
INSTALL(FILES ${css_files}
	DESTINATION ${DATA_INSTALL_DIR}/kmymoney2/html
)
INSTALL(DIRECTORY images
	DESTINATION ${DATA_INSTALL_DIR}/kmymoney2/html/
	PATTERN "CVS" EXCLUDE
	PATTERN "*Make*" EXCLUDE
)




#original Makefile.am contents follow:

#KDE_OPTIONS = noautodist
#
#SUBDIRS = images
#
#HOMEPAGES = home.html home_fr.fr.html home_de.de.html home_ru.ru.html home_br.pt_BR.html home_gl.html home_it.html
#
#WHATSNEWPAGES = whats_new.html whats_new_gl.html whats_new_de.html whats_new_it.html whats_new_fr.html whats_new_ru.html
#
#EXTRA_DIST = $(HOMEPAGES) $(WHATSNEWPAGES) kmymoney2.css welcome.css
#
#LOCAL_DIR = kmymoney2/html
#
#install-data-local:
#	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/$(LOCAL_DIR)
#	for file in $(EXTRA_DIST); do \
#	  $(INSTALL_DATA) $(srcdir)/$$file $(DESTDIR)$(kde_datadir)/$(LOCAL_DIR); \
#	done
#
#uninstall-local:
#	for file in $(EXTRA_DIST); do \
#	  rm -f $(DESTDIR)$(kde_datadir)/$(LOCAL_DIR)/$$file; \
#	done
#
