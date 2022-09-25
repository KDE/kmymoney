kdoctools_create_handbook(index.docbook INSTALL_DESTINATION ${KDE_INSTALL_DOCBUNDLEDIR}/en SUBDIR kmymoney)
if(UNIX AND NOT APPLE)
    kdoctools_create_manpage(man-kmymoney.1.docbook 1 INSTALL_DESTINATION ${MAN_INSTALL_DIR})
endif()