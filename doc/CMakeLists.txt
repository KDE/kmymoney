kdoctools_create_handbook(index.docbook INSTALL_DESTINATION ${KDE_INSTALL_DOCBUNDLEDIR}/en SUBDIR kmymoney)
kdoctools_create_manpage(man-kmymoney.1.docbook 1 INSTALL_DESTINATION ${KDE_INSTALL_MANDIR})

if(ENABLE_DOCBOOK_DOCS)
    # copy source files from doc directory into associated binary directory
    # which is required to run generated xml docs from build directory
    COPYDIR(
        SOURCES doc
        TYPES *.png *.svg *.css
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
        RETURN_SOURCE_NAMES HTML_SOURCE_FILES
    )

    file(GLOB DOCBOOK_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.docbook")

    # generate docbook output
    add_docbook(index
        TARGET_NAME po-en-docs-kmymoney-index-html
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
        SOURCE index.docbook
        DEPENDS ${HTML_SOURCE_FILES} ${DOCBOOK_SOURCE_FILES}
        FORMATS html
        CSS_FILE kmymoney.css
        INSTALL_DIR ${KDE_INSTALL_DOCBUNDLEDIR}/en/kmymoney
        RETURN_SOURCE_NAMES SOURCE_FILES
    )

    install(FILES ${DOC_DATA} DESTINATION ${KDE_INSTALL_DOCBUNDLEDIR}/en/kmymoney)
endif()
