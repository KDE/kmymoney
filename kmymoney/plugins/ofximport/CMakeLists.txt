# patch the version with the version defined in the build system
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmm_ofximport.desktop ${CMAKE_CURRENT_BINARY_DIR}/kmm_ofximport.desktop)
add_subdirectory( dialogs )
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dialogs
                    ${CMAKE_CURRENT_BINARY_DIR}/dialogs
                    ${LIBOFX_INCLUDE_DIR})

########### next target ###############

set(kmm_ofximport_PART_SRCS ofximporterplugin.cpp ofxpartner.cpp)

set(kmm_ofximport_PART_UI importoption.ui)

qt5_wrap_ui(kmm_ofximport_PART_SRCS ${kmm_ofximport_PART_UI})

add_library(kmm_ofximport MODULE ${kmm_ofximport_PART_SRCS})

target_link_libraries(kmm_ofximport
                      kmm_ofximport_dialogs
                      kmm_mymoney
                      kmm_widgets
                      kmm_plugin
                      ${KDE4_KDEUI_LIBS}
                      KF5::KIOWidgets
                      Alkimia::alkimia
                      ${LIBOFX_LIBRARIES})

########### install files ###############

install(FILES kmm_ofximport.rc
        DESTINATION  ${DATA_INSTALL_DIR}/kmm_ofximport)

install(TARGETS kmm_ofximport
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_ofximport.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
