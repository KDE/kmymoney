ADD_SUBDIRECTORY( phb ) 

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############




#original Makefile.am contents follow:

#KDE_OPTIONS = noautodist
#
######## kdevelop will overwrite this part!!! (begin)##########
#
#SUBDIRS = phb 
#
######## kdevelop will overwrite this part!!! (end)############
