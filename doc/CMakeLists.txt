kdoctools_create_handbook(index.docbook INSTALL_DESTINATION ${KDE_INSTALL_DOCBUNDLEDIR}/en SUBDIR kmymoney)
kdoctools_create_manpage(man-kmymoney.1.docbook 1 INSTALL_DESTINATION ${KDE_INSTALL_MANDIR})

if(ENABLE_DOCBOOK_HTML_FOUND)
    add_docbook_manual(
        TARGET_NAME doc-en-index-html
        DOCBOOK_SRC ${CMAKE_SOURCE_DIR}/doc/index.docbook
        SRC_DIR ${CMAKE_SOURCE_DIR}/doc
        CUSTOM_CSS kmymoney.css
        OUTPUT_DIR ${CMAKE_BINARY_DIR}/doc/en/kmymoney
        INSTALL_DIR ${KDE_INSTALL_DOCBUNDLEDIR}/en/kmymoney
    )
    list(APPEND ALL_MANUAL_TARGETS doc-en-index-html)

    file(GLOB LANG_DIRS RELATIVE ${CMAKE_SOURCE_DIR}/po ${CMAKE_SOURCE_DIR}/po/*)
    foreach(lang ${LANG_DIRS})
        set(SOURCE ${CMAKE_SOURCE_DIR}/po/${lang}/docs/kmymoney/index.docbook)
        if(EXISTS ${SOURCE})
            message(STATUS ${lang})
            add_docbook_manual(
                TARGET_NAME doc-${lang}-index-html
                DOCBOOK_SRC ${SOURCE}
                SRC_DIR ${CMAKE_SOURCE_DIR}/po/${lang}/docs/kmymoney
                CUSTOM_CSS kmymoney.css
                OUTPUT_DIR ${CMAKE_BINARY_DIR}/po/${lang}/docs/kmymoney
                INSTALL_DIR ${KDE_INSTALL_DOCBUNDLEDIR}/${lang}/kmymoney
            )
            list(APPEND ALL_MANUAL_TARGETS doc-${lang}-index-html)
        endif()
    endforeach()
    # Create an aggregate target that depends on all manual targets
    add_custom_target(doc-html ALL
        DEPENDS ${ALL_MANUAL_TARGETS}
        COMMENT "Build all DocBook HTML manuals"
    )
endif()
