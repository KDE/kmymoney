# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmm_kbanking.desktop ${CMAKE_CURRENT_BINARY_DIR}/kmm_kbanking.desktop)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
                    ${CMAKE_CURRENT_BINARY_DIR}/dialogs
                    ${CMAKE_CURRENT_SOURCE_DIR}/views
                    ${CMAKE_CURRENT_BINARY_DIR}/views
                    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
                    ${CMAKE_CURRENT_BINARY_DIR}/widgets
                    ${GWENHYWFAR_INCLUDE_DIRS}
                    ${AQBANKING_INCLUDE_DIRS})

add_subdirectory( dialogs )
add_subdirectory( views )
add_subdirectory( widgets )

########### next target ###############

set(kmm_kbanking_PART_SRCS
    mymoneybanking.cpp
    banking.cpp
    )

kde4_add_plugin(kmm_kbanking ${kmm_kbanking_PART_SRCS})

target_link_libraries(kmm_kbanking
                      kmm_kbanking_dialogs
                      kmm_kbanking_views
                      kmm_kbanking_widgets
                      kmm_mymoney
                      kmm_widgets
                      kmm_plugin
                      ${KDE4_KDEUI_LIBS}
                      ${GWENHYWFAR_LIBRARIES}
                      ${AQBANKING_LIBRARIES}
                      ${LIBALKIMIA_LIBRARY}
                      )

########### install files ###############

install(FILES kmm_kbanking.rc
        DESTINATION  ${DATA_INSTALL_DIR}/kmm_kbanking)

install(TARGETS kmm_kbanking
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_kbanking.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
