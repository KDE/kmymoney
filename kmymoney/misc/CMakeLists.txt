install(PROGRAMS financequote.pl
        DESTINATION ${KDE_INSTALL_DATADIR}/kmymoney/misc)

set( kmm_utils_validators_SRCS
  charvalidator.cpp
  validators.cpp
)

set( kmm_url_SRCS
  kmmurl.cpp
)

set( kmm_textbrowser_SRCS
  kmmtextbrowser.cpp
)
set( kmm_webconnect_SRCS
  webconnect.cpp
  kmmonlinequotesprofilemanager.cpp
)

set ( kmm_keychain_SRCS
  kmmkeychain.cpp
)

set ( kmm_yesno_SRCS
  kmmyesno.cpp
)

if( "${HAVE_UNISTD_H}" AND "${HAVE_PWD_H}" )
  set( kmm_utils_platformtools_SRCS
    platformtools_gnu.cpp
  )
elseif( "${HAVE_WINDOWS_H}" AND "${HAVE_LMCONS_H}" AND "${HAVE_PROCESS_H}" )
  set( kmm_utils_platformtools_SRCS
    platformtools_nognu.cpp
  )
else()
  message(FATAL_ERROR "make sure either windows.h and lmcons.h (on msvc platforms) or unistd.h and pwd.h (on other platforms) are present.")
endif()

set( kmm_utils_validators_HEADER
  charvalidator.h
  validators.h
)

set( kmm_webconnect_HEADER
  webpricequote.h
  webconnect.h
)

set ( kmm_keychain_HEADER
  kmmkeychain.h
)

set( kmm_utils_platformtools_HEADER
  platformtools.h
)

set( kmm_url_HEADER
  kmmurl.h
)

set( kmm_textbrowser_HEADER
  kmmtextbrowser.h
)

set( kmm_printer_SRCS
  kmm_printer.cpp
)

set( kmm_selections_SRCS
  selectedobjects.cpp
)

set( kmm_menuactionexchanger_SRCS
  kmm_menuactionexchanger.cpp
)

add_library(kmm_utils_validators STATIC ${kmm_utils_validators_SRCS})
add_library(kmm_utils_platformtools STATIC ${kmm_utils_platformtools_SRCS})
add_library(kmm_url STATIC ${kmm_url_SRCS})
add_library(kmm_textbrowser STATIC ${kmm_textbrowser_SRCS})
add_library(kmm_webconnect ${kmm_webconnect_SRCS})
add_library(kmm_keychain ${kmm_keychain_SRCS})
add_library(kmm_printer ${kmm_printer_SRCS})
add_library(kmm_selections ${kmm_selections_SRCS})
add_library(kmm_yesno ${kmm_yesno_SRCS})
add_library(kmm_menuactionexchanger ${kmm_menuactionexchanger_SRCS})

target_link_libraries( kmm_utils_validators
  PUBLIC
  Qt::Core
  Qt::Gui
)

target_link_libraries( kmm_webconnect
  PUBLIC
    Qt::Core
    Qt::Network
    KF${QT_MAJOR_VERSION}::ConfigCore
    KF${QT_MAJOR_VERSION}::Service
    KF${QT_MAJOR_VERSION}::Codecs
    KF${QT_MAJOR_VERSION}::KIOCore
    KF${QT_MAJOR_VERSION}::KIOWidgets
    Alkimia::alkimia
    kmm_mymoney
    kmm_csvimportercore
  PRIVATE
    KF${QT_MAJOR_VERSION}::I18n
)

target_link_libraries( kmm_keychain
  PUBLIC
  Qt::Core
  qt${QT_MAJOR_VERSION}keychain
)

target_link_libraries( kmm_utils_platformtools
  PUBLIC
  Qt::Core
)

target_link_libraries( kmm_url
  PUBLIC
  Qt::Core
)

target_link_libraries( kmm_textbrowser
  PUBLIC
  Qt::Core
  Qt::Widgets
)

target_link_libraries(kmm_printer
  PUBLIC
  Qt::Core
  Qt::PrintSupport
)

target_link_libraries( kmm_selections
  PUBLIC
  Qt::Core
)

target_link_libraries(kmm_yesno
  PRIVATE
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::WidgetsAddons
)

target_link_libraries(kmm_menuactionexchanger
  PUBLIC
    Qt::Core
    Qt::Widgets
)

set_target_properties(kmm_selections PROPERTIES
  VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
)
generate_export_header(kmm_selections)

set_target_properties(kmm_printer PROPERTIES
  VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
)
generate_export_header(kmm_printer)

set_target_properties(kmm_webconnect PROPERTIES
  VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
)
generate_export_header(kmm_webconnect)

set_target_properties(kmm_keychain PROPERTIES
        VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
)
generate_export_header(kmm_keychain)

set_target_properties(kmm_yesno PROPERTIES
  VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
)
generate_export_header(kmm_yesno)

set_target_properties(kmm_menuactionexchanger PROPERTIES
  VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
)
generate_export_header(kmm_menuactionexchanger)


set( kmm_gpgfile_SRCS
kgpgfile.cpp
)
add_library(kmm_gpgfile ${kmm_gpgfile_SRCS})

target_link_libraries( kmm_gpgfile
PUBLIC
Qt::Core
$<$<BOOL:${Gpgmepp_FOUND}>:Gpgmepp>
$<$<BOOL:${QGpgme_FOUND}>:QGpgme>
$<$<BOOL:${GpgmeppQt6_FOUND}>:GpgmeppQt6>
)
set_target_properties(kmm_gpgfile PROPERTIES
VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
)
generate_export_header(kmm_gpgfile)

install(TARGETS kmm_gpgfile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_gpgfile_export.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kmymoney COMPONENT Devel)

install(TARGETS
        kmm_printer
        kmm_webconnect
        kmm_keychain
        kmm_selections
        kmm_yesno
        kmm_menuactionexchanger
        ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/kmm_printer_export.h
        ${CMAKE_CURRENT_BINARY_DIR}/kmm_selections_export.h
        ${CMAKE_CURRENT_BINARY_DIR}/kmm_webconnect_export.h
        ${CMAKE_CURRENT_BINARY_DIR}/kmm_keychain_export.h
        ${CMAKE_CURRENT_BINARY_DIR}/kmm_yesno_export.h
        ${CMAKE_CURRENT_BINARY_DIR}/kmm_menuactionexchanger_export.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kmymoney COMPONENT Devel)

