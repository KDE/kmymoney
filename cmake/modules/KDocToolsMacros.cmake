
function(kmm_kdoctools_install podir)
    # Parse arguments
    set(options)
    set(oneValueArgs)
    set(multiValueArgs EXCLUDE_LANGUAGES)
    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    file(GLOB lang_dirs "${podir}/*")
    if (NOT KDE_INSTALL_MANDIR)
        if (MAN_INSTALL_DIR) # TODO KF6: deprecated, remove
            set(KDE_INSTALL_MANDIR ${MAN_INSTALL_DIR})
        else()
            set(KDE_INSTALL_MANDIR share/man)
        endif()
    endif()
    if (NOT KDE_INSTALL_DOCBUNDLEDIR)
        if (HTML_INSTALL_DIR) # TODO KF6: deprecated, remove
            set(KDE_INSTALL_DOCBUNDLEDIR ${HTML_INSTALL_DIR})
        else()
            set(KDE_INSTALL_DOCBUNDLEDIR share/doc/HTML)
        endif()
    endif()
    foreach(lang_dir ${lang_dirs})
        get_filename_component(lang ${lang_dir} NAME)

        if(lang IN_LIST ARGS_EXCLUDE_LANGUAGES)
            message(WARNING "Translation for documentation for '${lang}' excluded")
            continue()
        endif()

        file(GLOB_RECURSE docbooks RELATIVE "${lang_dir}" "${lang_dir}/docs/*.docbook")
        foreach(docbook ${docbooks})
            string(REGEX MATCH "\\.([0-9])\\.docbook" match ${docbook})
            if (match)
                kdoctools_create_manpage("${lang_dir}/${docbook}" ${CMAKE_MATCH_1}
                    INSTALL_DESTINATION ${KDE_INSTALL_MANDIR}/${lang}
                )
            else()
                string(REGEX MATCH "^docs/(.*)/index.docbook" match ${docbook})
                if (match)
                    kdoctools_create_handbook("${lang_dir}/${docbook}"
                        INSTALL_DESTINATION ${KDE_INSTALL_DOCBUNDLEDIR}/${lang}
                        SUBDIR ${CMAKE_MATCH_1}
                    )
                endif()
            endif()
        endforeach()
    endforeach()
endfunction()
